#include "Environment.h"

namespace Canal {

Environment::Environment(const llvm::Module &module)
    : mModule(module), mTargetData(&module), mSlotTracker(module)
{
}

} // namespace Canal
