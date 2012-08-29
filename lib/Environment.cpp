#include "Environment.h"
#include <llvm/Module.h>

namespace Canal {

Environment::Environment(const llvm::Module &module)
    : mModule(module), mTargetData(&module), mSlotTracker(module)
{
}

llvm::LLVMContext &
Environment::getContext() const
{
    return mModule.getContext();
}

} // namespace Canal
