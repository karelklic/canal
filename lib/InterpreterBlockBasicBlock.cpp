#include "InterpreterBlockBasicBlock.h"
#include "Constructors.h"
#include "Environment.h"
#include "Utils.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Type.h>
#include <sstream>

namespace Canal {
namespace InterpreterBlock {

BasicBlock::BasicBlock(const llvm::BasicBlock &basicBlock,
                       const Constructors &constructors)
    : mBasicBlock(basicBlock), mEnvironment(constructors.getEnvironment())
{
}

std::string
BasicBlock::toString() const
{
    SlotTracker &slotTracker = mEnvironment.getSlotTracker();
    slotTracker.setActiveFunction(*mBasicBlock.getParent());

    std::stringstream ss;
    ss << "*** basicBlock ";
    if (mBasicBlock.hasName())
        ss << mBasicBlock.getName().str();
    else
        ss << "<label>:" << slotTracker.getLocalSlot(mBasicBlock);
    ss << std::endl;

    llvm::BasicBlock::const_iterator it = mBasicBlock.begin();
    for (; it != mBasicBlock.end(); ++it)
    {
        if (it->getType()->isVoidTy())
            continue;

        ss << mOutputState.toString(*it, slotTracker);
    }

    return ss.str();
}

} // namespace InterpreterBlock
} // namespace Canal
