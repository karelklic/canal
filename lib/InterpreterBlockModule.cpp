#include "InterpreterBlockModule.h"
#include "InterpreterBlockFunction.h"
#include "State.h"
#include "Pointer.h"
#include "Utils.h"
#include "Constructors.h"
#include <llvm/Module.h>

namespace Canal {
namespace InterpreterBlock {

Module::Module(const llvm::Module &module,
               const Constructors &constructors)
    : mModule(module)
{
    // Prepare the state with all globals.  Global pointers are
    // allocated automatically -- they point to globals section.
    State globalState;
    {
        llvm::Module::const_global_iterator it = module.global_begin(),
            itend = module.global_end();

        for (; it != itend; ++it)
        {
            if (it->isConstant() && it->hasInitializer())
            {
                Domain *value = constructors.create(*it->getInitializer(), NULL);
                globalState.addGlobalVariable(*it, value);
                continue;
            }

            const llvm::Type &elementType = *it->getType()->getElementType();
            Domain *block = constructors.create(elementType);
            globalState.addGlobalBlock(*it, block);

            Domain *value = constructors.create(*it->getType());

            Pointer::InclusionBased &pointer =
                dynCast<Pointer::InclusionBased&>(*value);

            pointer.addTarget(Pointer::Target::GlobalBlock,
                              it,
                              it,
                              std::vector<Domain*>(),
                              NULL);

            globalState.addGlobalVariable(*it, value);
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
            function->getInputState().merge(globalState);
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
        if (&(*it)->getFunction() == &function)
            return *it;
    }

    return NULL;
}

} // namespace InterpreterBlock
} // namespace Canal
