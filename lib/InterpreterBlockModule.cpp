#include "InterpreterBlockModule.h"
#include "InterpreterBlockFunction.h"
#include "State.h"
#include "Pointer.h"
#include "Utils.h"
#include "Constructors.h"
#include "Environment.h"
#include <llvm/Module.h>
#include <llvm/Constants.h>
#include <sstream>

namespace Canal {
namespace InterpreterBlock {

Module::Module(const llvm::Module &module,
               const Constructors &constructors)
    : mModule(module), mEnvironment(constructors.getEnvironment())
{
    // Prepare the state with all globals.  Global pointers are
    // allocated automatically -- they point to globals section.
    {
        tsortInit();

        for (const llvm::GlobalVariable* it = tsortNext(); it != NULL; it = tsortNext())
        {
            if (it->isConstant() && it->hasInitializer())
            {
                Domain *value = constructors.create(*it->getInitializer(), &mGlobalState);
                mGlobalState.addGlobalVariable(*it, value);
                continue;
            }

            const llvm::Type &elementType = *it->getType()->getElementType();
            Domain *block = constructors.create(elementType);
            mGlobalState.addGlobalBlock(*it, block);

            Domain *value = constructors.create(*it->getType());

            Pointer::InclusionBased &pointer =
                dynCast<Pointer::InclusionBased&>(*value);

            pointer.addTarget(Pointer::Target::Block,
                              it,
                              it,
                              std::vector<Domain*>(),
                              NULL);

            mGlobalState.addGlobalVariable(*it, value);
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
    std::vector<Function*>::const_iterator it = mFunctions.begin();
    for (; it != mFunctions.end(); ++it)
        delete *it;
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
    std::stringstream ss;
    ss << "***************************************" << std::endl;
    ss << "* module " << mModule.getModuleIdentifier() << std::endl;
    ss << "***************************************" << std::endl;

    // Print globals.
    {
        SlotTracker &slotTracker = mEnvironment.getSlotTracker();
        llvm::Module::const_global_iterator it = mModule.global_begin(),
            itend = mModule.global_end();

        for (; it != itend; ++it)
            ss << mGlobalState.toString(*it, slotTracker);

        if (mModule.global_begin() != itend)
            ss << std::endl;
    }

    // Print functions.
    std::vector<Function*>::const_iterator it = mFunctions.begin();
    for (; it != mFunctions.end(); ++it)
    {
        ss << std::endl;
        ss << (*it)->toString();
    }

    return ss.str();
}

void
Module::updateGlobalState()
{
    std::vector<Function*>::const_iterator it = mFunctions.begin();
    for (; it != mFunctions.end(); ++it)
    {
        // Merge global blocks, global variables.  Merge function
        // blocks that do not belong to this function.  Merge returned
        // value.
        mGlobalState.mergeGlobal((*it)->getOutputState());
    }
}

//Init topological sort
void
Module::tsortInit()
{
    llvm::Module::const_global_iterator it = mModule.global_begin(),
        itend = mModule.global_end();
    tsortValue* dependency = new tsortValue;
    dependency->count = 0;

    //Find dependencies for all variables
    for (; it != itend; ++it)
    {
        if (dependency->count) { //Lazy allocation - allocate new only if last one was used
            dependency = new tsortValue;
            dependency->count = 0;
        }
        dependency->constant = &(*it);
        if (!tsortDepend(*it, dependency)) {
            mTsortReady.push_back(&(*it));
        }
    }
    if (dependency->count == 0) delete dependency;
}

bool
Module::tsortDepend(const llvm::GlobalVariable& what, tsortValue* value) {
    if (!what.isConstant() || !what.hasInitializer()) return false;
    return tsortDepend(*what.getInitializer(), value);
}

bool
Module::tsortDepend(const llvm::Constant& what, tsortValue* value) {
    bool retval = false;
    if (llvm::isa<llvm::ConstantExpr>(what))
    {
        mTsortDependencies[*what.op_begin()].push_back(value);
        value->count ++;
        return true;
    }

    if (llvm::isa<llvm::ConstantStruct>(what))
    {
        const llvm::ConstantStruct &structValue = llvmCast<llvm::ConstantStruct>(what);
        uint64_t elementCount = structValue.getType()->getNumElements();

        for (uint64_t i = 0; i < elementCount; ++i)
            retval |= tsortDepend(*structValue.getOperand(i), value);

        return retval;
    }

    if (llvm::isa<llvm::ConstantArray>(what))
    {
        const llvm::ConstantArray &arrayValue = llvmCast<llvm::ConstantArray>(what);
        uint64_t elementCount = arrayValue.getType()->getNumElements();

        for (uint64_t i = 0; i < elementCount; ++i)
           retval |= tsortDepend(*arrayValue.getOperand(i), value);

        return retval;
    }
    return false;
}

void
Module::tsortDecrement(tsortValue *&value) {
    if (value->count == 1) {
        const llvm::GlobalVariable* constant = llvmCast<llvm::GlobalVariable>(value->constant);
        mTsortReady.push_back(constant);
        delete value;
        value = NULL;
    }
    else value->count--;
}

const llvm::GlobalVariable*
Module::tsortNext()
{
    if (!mTsortReady.size()) {
        CANAL_ASSERT_MSG(mTsortDependencies.size() == 0, "Circular dependencies among global variables");
        return NULL;
    }

    const llvm::GlobalVariable* ret = mTsortReady.back();
    mTsortReady.pop_back();
    const std::map<const llvm::Value*, std::vector<tsortValue*> >::iterator dependent = mTsortDependencies.find(ret);

    if (dependent != mTsortDependencies.end()) { //Handle dependent constants
        for (std::vector<tsortValue*>::iterator it = dependent->second.begin(); it != dependent->second.end(); it ++) {
            tsortDecrement(*it);
        }
        mTsortDependencies.erase(dependent);
    }
    return ret;
}

} // namespace InterpreterBlock
} // namespace Canal
