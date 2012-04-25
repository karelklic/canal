#include "Stack.h"

namespace Canal {

StackFrame::StackFrame(const llvm::Function &function, State &initialState)
{
    llvm::Function::const_iterator it = function.begin(), itend = function.end();
    for (; it != itend; ++it)
        mBlockInputState[it] = state;

    startBlock(function.begin());
}

bool
Stack::nextInstruction()
{
    return ++mCurrentInstruction == mCurrentBlock->end() ? nextBlock() : true;
}

bool
Stack::nextBlock()
{
    if (mCurrentState != mBlockOutputState[mCurrentBlock])
    {
        mChanged = true;
        mBlockOutputState[mCurrentBlock] = mCurrentState;
    }

    if (++mCurrentBlock == mFunction.end())
    {
        if (mChanged)
        {
            mChanged = false;
            startBlock(function.begin());
        }
        else
        {
            // We reached a fixpoint.
            return false;
        }
    }
    // Interpreting next block is necessary to find a fixpoint for
    // this function.
    return true;
}

void
StackFrame::startBlock(llvm::Function::const_iterator block)
{
    mCurrentBlock = block;

    // Merge out states of predecessors to input state of
    // current block.
    llvm::const_pred_iterator it = llvm::pred_begin(block), itend = llvm::pred_end(block);
    for (; it != itend; ++it)
    {
        CANAL_ASSERT(&*it != &block->getParent()->getEntryBlock() && "Entry block cannot have predecessors!");
        mBlockInputState[block].merge(mBlockOutputState[*it]);
    }

    mCurrentState = mBlockInputState[block];
    mCurrentInstruction = block->begin();
}

} // namespace Canal
