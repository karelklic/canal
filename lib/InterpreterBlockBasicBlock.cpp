#include "InterpreterBlockBasicBlock.h"
#include "Utils.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>

namespace Canal {
namespace InterpreterBlock {

BasicBlock::BasicBlock(const llvm::BasicBlock &basicBlock,
                       const Constructors &constructors)
    : mBasicBlock(basicBlock)
{
    // TODO: create bottom values for every instruction.  Both input
    // and output state.
}

void
BasicBlock::mergeOutputToInput(const BasicBlock &basicBlock)
{
    CANAL_ASSERT_MSG(&mBasicBlock != &mBasicBlock.getParent()->getEntryBlock(),
                     "Entry block cannot have predecessors!");

    mInputState.merge(basicBlock.mOutputState);
}

} // namespace InterpreterBlock
} // namespace Canal
