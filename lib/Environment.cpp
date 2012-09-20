#include "Environment.h"
#include <llvm/Module.h>

namespace Canal {

Environment::Environment(const llvm::Module *module)
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
