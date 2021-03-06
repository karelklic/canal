#include "InterpreterBasicBlock.h"
#include "Constructors.h"
#include "Environment.h"
#include "Utils.h"

namespace Canal {
namespace Interpreter {

BasicBlock::BasicBlock(const llvm::BasicBlock &basicBlock,
                       const Constructors &constructors)
    : mBasicBlock(basicBlock), mEnvironment(constructors.getEnvironment())
{
}

size_t
BasicBlock::memoryUsage() const
{
    size_t result = sizeof(BasicBlock);
    result += mInputState.memoryUsage();
    result += mOutputState.memoryUsage();
    return result;
}

std::string
BasicBlock::toString() const
{
    SlotTracker &slotTracker = mEnvironment.getSlotTracker();
    slotTracker.setActiveFunction(*mBasicBlock.getParent());

    StringStream ss;
    ss << "*** basicBlock ";
    if (mBasicBlock.hasName())
        ss << mBasicBlock.getName();
    else
        ss << "<label>:" << slotTracker.getLocalSlot(mBasicBlock);

    ss << "\n";

    llvm::BasicBlock::const_iterator it = mBasicBlock.begin();
    for (; it != mBasicBlock.end(); ++it)
    {
        if (it->getType()->isVoidTy())
            continue;

        ss << mOutputState.toString(*it, slotTracker);
    }

    return ss.str();
}

} // namespace Interpreter
} // namespace Canal
