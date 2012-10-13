#include "InterpreterBlockModule.h"
#include "InterpreterBlockFunction.h"
#include "State.h"
#include "Pointer.h"
#include "Utils.h"
#include "Constructors.h"
#include "Environment.h"
#include <llvm/Module.h>
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
        llvm::Module::const_global_iterator it = module.global_begin(),
            itend = module.global_end();

        for (; it != itend; ++it)
        {
            if (it->isConstant() && it->hasInitializer())
            {
                Domain *value = constructors.create(*it->getInitializer(), NULL);
                mGlobalState.addGlobalVariable(*it, value);
                continue;
            }

            const llvm::Type &elementType = *it->getType()->getElementType();
            Domain *block = constructors.create(elementType);
            mGlobalState.addGlobalBlock(*it, block);

            Domain *value = constructors.create(*it->getType());

            Pointer::InclusionBased &pointer =
                dynCast<Pointer::InclusionBased&>(*value);

            pointer.addTarget(Pointer::Target::GlobalBlock,
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

} // namespace InterpreterBlock
} // namespace Canal
