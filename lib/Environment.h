#ifndef LIBCANAL_ENVIRONMENT_H
#define LIBCANAL_ENVIRONMENT_H

#include "SlotTracker.h"

namespace Canal {

class Constructors;

class Environment
{
    llvm::Module *mModule;

    llvm::TargetData mTargetData;

    mutable SlotTracker mSlotTracker;

    Constructors *mConstructors;

public:
    // @param module
    //   LLVM module that contains all functions.
    Environment(llvm::Module *module);
    ~Environment();

    llvm::LLVMContext &getContext() const;

    llvm::Module &getModule() const { return *mModule; }

    const llvm::TargetData &getTargetData() const { return mTargetData; }

    SlotTracker &getSlotTracker() const { return mSlotTracker; }

    const Constructors &getConstructors() const
    {
        return *mConstructors;
    }

public:
    void setConstructors(Constructors *constructors)
    {
        mConstructors = constructors;
    }
};

} // namespace Canal

#endif // LIBCANAL_ENVIRONMENT_H
