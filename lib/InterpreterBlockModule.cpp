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
    State state;
    {
        llvm::Module::const_global_iterator it = module.global_begin(),
            itend = module.global_end();

        for (; it != itend; ++it)
        {
            if (it->isConstant())
            {
                llvm::Constant &constant = llvmCast<llvm::Constant>(*it);
                Domain *value = constructors.create(constant);
                state.addGlobalVariable(*it, value);
                continue;
            }

            const llvm::Type &elementType = *it->getType()->getElementType();
            Domain *block = constructors.create(elementType);
            state.addGlobalBlock(*it, block);

            Domain *value = constructors.create(*it->getType());

            Pointer::InclusionBased &pointer =
                dynCast<Pointer::InclusionBased&>(*value);

            pointer.addTarget(Pointer::Target::GlobalBlock,
                              it,
                              it,
                              std::vector<Domain*>(),
                              NULL);

            state.addGlobalVariable(*it, value);
        }
    }

    // Prepare the initial state of all functions.
    {
        llvm::Module::const_iterator it = module.begin(),
            itend = module.end();
        for (; it != itend; ++it)
            mFunctions.push_back(new Function(*it, constructors));
    }
}


Module::~Module()
{
    std::vector<Function*>::const_iterator it = mFunctions.begin();
    for (; it != mFunctions.end(); ++it)
        delete *it;
}

} // namespace InterpreterBlock
} // namespace Canal
