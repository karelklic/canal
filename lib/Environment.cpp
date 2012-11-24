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

uint64_t
Environment::getTypeStoreSize(const llvm::Type &type) const
{
    llvm::Type &modifiableType = const_cast<llvm::Type&>(type);
    uint64_t size = mTargetData.getTypeStoreSize(&modifiableType);
    CANAL_ASSERT_MSG(mTargetData.getTypeAllocSize(&modifiableType) == size,
                     "TODO: handle different alloc and store size");

    return size;
}

} // namespace Canal
