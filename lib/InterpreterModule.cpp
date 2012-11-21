#include "InterpreterModule.h"
#include "InterpreterFunction.h"
#include "State.h"
#include "Pointer.h"
#include "PointerUtils.h"
#include "Utils.h"
#include "Constructors.h"
#include "Environment.h"
#include <set>

namespace Canal {
namespace Interpreter {

/** Helper function for getSortedGlobalVariables().
 */
static void
processValue(const llvm::Value *value,
             std::vector<const llvm::GlobalVariable*> &sorted,
             std::set<const llvm::GlobalVariable*> &inserted,
             std::set<const llvm::GlobalVariable*> &visited)
{
    const llvm::GlobalVariable *variable =
        dynCast<const llvm::GlobalVariable*>(value);

    if (!variable)
        return;

    if (!variable->isConstant() ||
        !variable->hasInitializer())
    {
        inserted.insert(variable);
        sorted.push_back(variable);
        return;
    }

    CANAL_ASSERT_MSG(visited.find(variable) == visited.end(),
                     "Circular dependencies among global variables");

    visited.insert(variable);

    const llvm::Constant *initializer = variable->getInitializer();
    llvm::Constant::const_op_iterator it = initializer->op_begin(),
        itend = initializer->op_end();

    for (; it != itend; ++it)
        processValue(it->get(), sorted, inserted, visited);

    inserted.insert(variable);
    sorted.push_back(variable);
}

static std::vector<const llvm::GlobalVariable*>
getSortedGlobalVariables(const llvm::Module &module)
{
    // Find dependencies for all variables
    llvm::Module::const_global_iterator it = module.global_begin(),
        itend = module.global_end();

    std::vector<const llvm::GlobalVariable*> sorted;
    std::set<const llvm::GlobalVariable*> inserted, visited;
    for (; it != itend; ++it)
        processValue(it, sorted, inserted, visited);

    return sorted;
}

Module::Module(const llvm::Module &module,
               const Constructors &constructors)
    : mModule(module), mEnvironment(constructors.getEnvironment())
{
    // Prepare the state with all globals.  Global pointers are
    // allocated automatically -- they point to globals section.
    {
        std::vector<const llvm::GlobalVariable*> sorted =
            getSortedGlobalVariables(module);

        std::vector<const llvm::GlobalVariable*>::const_iterator
            it = sorted.begin(), itend = sorted.end();

        for (; it != itend; ++it)
        {
            if ((*it)->isConstant() && (*it)->hasInitializer())
            {
                Domain *value = constructors.create(*(*it)->getInitializer(),
                                                    **it,
                                                    &mGlobalState);

                mGlobalState.addGlobalVariable(**it, value);
                continue;
            }

            const llvm::Type &elementType = *(*it)->getType()->getElementType();
            Domain *block = constructors.create(elementType);
            mGlobalState.addGlobalBlock(**it, block);

            Domain *value = constructors.create(*(*it)->getType());

            Pointer::Utils::addTarget(*value,
                                      Pointer::Target::Block,
                                      *it,
                                      *it,
                                      std::vector<Domain*>(),
                                      NULL);

            mGlobalState.addGlobalVariable(**it, value);
        }
    }

    // Prepare the initial state of all functions.
    {
        llvm::Module::const_iterator it = module.begin(),
            itend = module.end();

        for (; it != itend; ++it)
        {
            if (it->isDeclaration())
                continue;

            Function *function = new Function(*it, constructors);
            function->getInputState().merge(mGlobalState);
            mFunctions.push_back(function);
        }
    }
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

} // namespace Interpreter
} // namespace Canal
