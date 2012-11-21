#include "Environment.h"
#include "Utils.h"

namespace Canal {

Environment::Environment(llvm::Module *module)
    : mModule(module), mTargetData(module), mSlotTracker(*module)
{
    CANAL_ASSERT_MSG(module, "Module cannot be NULL");
}

Environment::~Environment()
{
    delete mModule;
}

llvm::LLVMContext &
Environment::getContext() const
{
    return mModule->getContext();
}

} // namespace Canal
