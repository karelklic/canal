#ifndef LIBCANAL_ENVIRONMENT_H
#define LIBCANAL_ENVIRONMENT_H

#include "SlotTracker.h"
#include <llvm/Target/TargetData.h>

namespace llvm {
class Module;
class LLVMContext;
} // namespace llvm

namespace Canal {

class Environment
{
protected:
    const llvm::Module *mModule;
    llvm::TargetData mTargetData;
    mutable SlotTracker mSlotTracker;

public:
    // @param module
    //   LLVM module that contains all functions.
    Environment(const llvm::Module *module);
    ~Environment();

    llvm::LLVMContext &getContext() const;
    const llvm::Module &getModule() const { return *mModule; }
    const llvm::TargetData &getTargetData() const { return mTargetData; }
    llvm::SlotTracker &getSlotTracker() const { return mSlotTracker; }
};

} // namespace Canal

#endif // LIBCANAL_ENVIRONMENT_H
