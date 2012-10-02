#include "InterpreterBlockBasicBlock.h"
#include "Constructors.h"
#include "Environment.h"
#include "Utils.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
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

    ss << "**** inputState" << std::endl;
    ss << mInputState.toString();
    ss << "**** outputState" << std::endl;
    ss << mOutputState.toString();
    return ss.str();
}

} // namespace InterpreterBlock
} // namespace Canal
