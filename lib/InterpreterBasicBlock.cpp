#include "InterpreterBasicBlock.h"
#include "Constructors.h"
#include "Environment.h"
#include "Utils.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Type.h>
#include <sstream>

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

} // namespace Interpreter
} // namespace Canal
