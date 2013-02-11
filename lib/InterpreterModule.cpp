#include "InterpreterModule.h"
#include "InterpreterFunction.h"
#include "MemoryState.h"
#include "MemoryPointer.h"
#include "MemoryUtils.h"
#include "Utils.h"
#include "Constructors.h"
#include "Environment.h"
#include <set>

namespace Canal {
namespace Interpreter {

Module::Module(const llvm::Module &module,
               const Constructors &constructors)
    : mModule(module), mEnvironment(constructors.getEnvironment())
{
    initializeGlobalState(constructors);
    initializeFunctions(constructors);
}

Module::~Module()
{
    llvm::DeleteContainerPointers(mFunctions);
}

Function *
Module::getFunction(const char *name) const
{
    llvm::StringRef nameString(name);
    std::vector<Function*>::const_iterator it = mFunctions.begin();
    for (; it != mFunctions.end(); ++it)
    {
        if ((*it)->getName().equals(nameString))
            return *it;
    }

    return NULL;
}

Function *
Module::getFunction(const llvm::Function &function) const
{
    std::vector<Function*>::const_iterator it = mFunctions.begin();
    for (; it != mFunctions.end(); ++it)
    {
        if (&(*it)->getLlvmFunction() == &function)
            return *it;
    }

    return NULL;
}

std::string
Module::toString() const
{
    StringStream ss;
    ss << "***************************************\n";
    ss << "* module " << mModule.getModuleIdentifier() << "\n";
    ss << "***************************************\n";

    // Print globals.
    SlotTracker &slotTracker = mEnvironment.getSlotTracker();
    llvm::Module::const_global_iterator git = mModule.global_begin(),
        gitend = mModule.global_end();

    for (; git != gitend; ++git)
        ss << mGlobalState.toString(*git, slotTracker);

    if (mModule.global_begin() != gitend)
        ss << "\n";

    // Print functions.
    std::vector<Function*>::const_iterator fit = mFunctions.begin(),
        fitend = mFunctions.end();

    for (; fit != fitend; ++fit)
        ss << "\n" << (*fit)->toString();

    return ss.str();
}

void
Module::updateGlobalState()
{
    std::vector<Function*>::const_iterator it = mFunctions.begin(),
        itend = mFunctions.end();

    for (; it != itend; ++it)
    {
        // Merge global blocks, global variables.  Merge function
        // blocks that do not belong to this function.  Merge returned
        // value.
        mGlobalState.mergeGlobal((*it)->getOutputState());
    }
}

/** Helper function for getSortedGlobalVariables().
 */
static void
processValue(const llvm::Value *value,
             std::vector<const llvm::GlobalVariable*> &sorted,
             const llvm::GlobalVariable *currentVariable)
{
    const llvm::GlobalVariable *variable = dynCast<llvm::GlobalVariable>(value);
    if (variable)
    {
        if (!variable->isConstant() ||
            !variable->hasInitializer())
        {
            sorted.push_back(variable);
        }
        else
        {
            CANAL_ASSERT_MSG(variable != currentVariable,
                             "Circular dependencies among global variables: " << *variable);

            const llvm::Constant *initializer = variable->getInitializer();
            llvm::Constant::const_op_iterator it = initializer->op_begin(),
                itend = initializer->op_end();

            for (; it != itend; ++it)
                processValue(it->get(), sorted, variable);

            sorted.push_back(variable);
        }
    }
    else
    {
        const llvm::Constant *constant = dynCast<llvm::Constant>(value);
        if (!constant)
            return;

        llvm::Constant::const_op_iterator it = constant->op_begin(),
            itend = constant->op_end();

        for (; it != itend; ++it)
            processValue(it->get(), sorted, currentVariable);
    }
}

static std::vector<const llvm::GlobalVariable*>
getSortedGlobalVariables(const llvm::Module &module)
{
    // Find dependencies for all variables
    llvm::Module::const_global_iterator it = module.global_begin(),
        itend = module.global_end();

    std::vector<const llvm::GlobalVariable*> sorted;
    std::set<const llvm::GlobalVariable*> processed;
    for (; it != itend; ++it)
        processValue(it, sorted, NULL);

    return sorted;
}

void
Module::initializeGlobalState(const Constructors &constructors)
{
    // Get a sorted list of global variables.  If a global variable
    // dependes another global variable, that global variable must be
    // processed before it is referenced.
    std::vector<const llvm::GlobalVariable*> sorted =
        getSortedGlobalVariables(mModule);

    std::vector<const llvm::GlobalVariable*>::const_iterator
        it = sorted.begin(), itend = sorted.end();

    for (; it != itend; ++it)
    {
        // Handle constant global variables.  Global variables are
        // placed to the variables map of state, no pointer + block is
        // created.
        //
        // Constant global variables have optional initializer.
        // An example of variable with an initializer:
        //   @.str30 = private unnamed_addr constant [6 x i8] c"bytes\00", align 1
        // An example of variable without an initializer:
        //   @is_basic_table = external constant [0 x i32]
        if ((*it)->isConstant())
        {
            Domain *value = NULL;
            if ((*it)->hasInitializer())
            {
                value = constructors.create(*(*it)->getInitializer(),
                                            &mGlobalState);
            }
            else
                value = constructors.create(*(*it)->getType());

            mGlobalState.addGlobalVariable(**it, value);
            continue;
        }

        // Non-constant global variables are placed to the state as a
        // memory block and a pointer to it in order to allow their
        // modification via the pointer.
        //
        // Non-constant global variables also have an optional initializer.
        // An example of a variable with an initializer:
        //   @have_read_stdin = internal global i8 0, align 1
        // An example of a variable without an initializer:
        //   @program_name = external global i8*
        //
        // First, we initialize a new memory block and add it to the
        // state.
        Domain *blockValue = NULL;
        if ((*it)->hasInitializer())
        {
            blockValue = constructors.create(*(*it)->getInitializer(),
                                             &mGlobalState);
        }
        else
        {
            const llvm::Type &elementType =
                *(*it)->getType()->getElementType();

            blockValue = constructors.create(elementType);
        }

        Memory::Block *block = new Memory::Block(Memory::Block::HeapMemoryType,
                                                 blockValue);

        mGlobalState.addBlock(**it, block);

        // Second, we initialize a pointer to the block and add it to
        // the state.
        Domain *value = constructors.create(*(*it)->getType());
        Memory::Utils::addTarget(*value,
                                 **it,
                                 NULL);

        mGlobalState.addGlobalVariable(**it, value);
    }
}

void
Module::initializeFunctions(const Constructors &constructors)
{
    // Prepare the initial state of all functions.
    llvm::Module::const_iterator it = mModule.begin(),
        itend = mModule.end();

    for (; it != itend; ++it)
    {
        if (it->isDeclaration())
            continue;

        Function *function = new Function(*it, constructors);
        function->getInputState().merge(mGlobalState);
        mFunctions.push_back(function);
    }
}

} // namespace Interpreter
} // namespace Canal
