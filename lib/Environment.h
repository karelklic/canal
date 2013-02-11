#ifndef LIBCANAL_ENVIRONMENT_H
#define LIBCANAL_ENVIRONMENT_H

#include "SlotTracker.h"

namespace Canal {

class Environment
{
    llvm::Module *mModule;

#if LLVM_VERSION_MAJOR > 2 && LLVM_VERSION_MINOR > 1
    llvm::DataLayout mTargetData;
#else
    llvm::TargetData mTargetData;
#endif

    mutable SlotTracker mSlotTracker;

    Constructors *mConstructors;

public:
    // @param module
    //   LLVM module that contains all functions.
    Environment(llvm::Module *module);

    ~Environment();

    llvm::LLVMContext &getContext() const;

    llvm::Module &getModule() const
    {
        return *mModule;
    }

#if LLVM_VERSION_MAJOR > 2 && LLVM_VERSION_MINOR > 1
    const llvm::DataLayout &getTargetData() const
#else
    const llvm::TargetData &getTargetData() const
#endif
    {
        return mTargetData;
    }

    SlotTracker &getSlotTracker() const
    {
        return mSlotTracker;
    }

    const Constructors &getConstructors() const
    {
        return *mConstructors;
    }

    void setConstructors(Constructors *constructors)
    {
        mConstructors = constructors;
    }

    uint64_t getTypeStoreSize(const llvm::Type &type) const;
};

} // namespace Canal

#endif // LIBCANAL_ENVIRONMENT_H
