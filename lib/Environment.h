#ifndef LIBCANAL_ENVIRONMENT_H
#define LIBCANAL_ENVIRONMENT_H

#include "SlotTracker.h"

namespace Canal {

class Constructors;

class Environment
{
    llvm::Module *mModule;

    // Renamed to DataLayout in LLVM 3.2.
    llvm::TargetData mTargetData;

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
