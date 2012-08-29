#ifndef LIBCANAL_ENVIRONMENT_H
#define LIBCANAL_ENVIRONMENT_H

#include "SlotTracker.h"
#include <llvm/Target/TargetData.h>

namespace llvm {
class Module;
} // namespace llvm

namespace Canal {

class Environment
{
public:
    const llvm::Module &mModule;
    llvm::TargetData mTargetData;
    mutable SlotTracker mSlotTracker;

public:
    // @param module
    //   LLVM module that contains all functions.
    Environment(const llvm::Module &module);
};

} // namespace Canal

#endif // LIBCANAL_ENVIRONMENT_H
