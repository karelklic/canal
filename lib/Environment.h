#ifndef LIBCANAL_ENVIRONMENT_H
#define LIBCANAL_ENVIRONMENT_H

namespace Canal {

class Environment
{
public:
    const llvm::Module &mModule;
    llvm::TargetData mTargetData;
    SlotTracker mSlotTracker;

public:
    // @param module
    //   LLVM module that contains all functions.
    Environment(const llvm::Module &module);

};

} // namespace Canal

#endif // LIBCANAL_ENVIRONMENT_H
