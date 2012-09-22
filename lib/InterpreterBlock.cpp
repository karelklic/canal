#include "InterpreterBlock.h"

namespace Canal {
namespace InterpreterBlock {

Interpreter::Interpreter(const llvm::Module *module)
    : mEnvironment(module)
{
    // Prepare the state with all globals.  Global pointers are
    // allocated automatically -- they point to globals section.
    State state;
    {
        llvm::Module::const_global_iterator it = module->global_begin(),
            itend = module->global_end();

        for (; it != itend; ++it)
        {
            if (it->isConstant())
            {
                llvm::Constant &constant = llvmCast<llvm::Constant>(*it);
                Domain *value = mConstructors.create(constant, environment);
                state.addGlobalVariable(*it, value);
                continue;
            }

            const llvm::Type &elementType = *it->getType()->getElementType();
            Domain *block = mConstructors.create(elementType, environment);
            state.addGlobalBlock(*it, block);

            Domain *value = mConstructors.create(*it->getType(), environment);

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
        llvm::Module::iterator it = module->begin(),
            itend = module->end();
        for (; it != itend; ++it)
            mFunctions.push_back(Function(*it));
    }
}

} // namespace InterpreterBlock
} // namespace Canal
