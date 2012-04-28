#include "Stack.h"
#include "Value.h"
#include "Utils.h"
#include <llvm/Instructions.h>
#include <llvm/Support/CFG.h>

namespace Canal {

StackFrame::StackFrame(const llvm::Function *function, const State &initialState) : mFunction(function)
{
    llvm::Function::const_iterator it = function->begin(), itend = function->end();
    for (; it != itend; ++it)
        mBlockInputState[it] = initialState;

    startBlock(function->begin());
}

bool
StackFrame::nextInstruction()
{
    return ++mCurrentInstruction == mCurrentBlock->end() ? nextBlock() : true;
}

Value *
StackFrame::getReturnedValue() const
{
    Value *mergedValue = NULL;
    llvm::Function::const_iterator it = mFunction->begin(), itend = mFunction->end();
    for (; it != itend; ++it)
    {
        if (!dynamic_cast<const llvm::ReturnInst*>(it->getTerminator()))
            continue;

        if (mergedValue)
            mergedValue->merge(*mBlockOutputState.find(it)->second.mReturnedValue);
        else
            mergedValue = mBlockOutputState.find(it)->second.mReturnedValue->clone();

    }

    return mergedValue;
}

void
StackFrame::mergeGlobalVariables(State &target) const
{
    llvm::Function::const_iterator it = mFunction->begin(), itend = mFunction->end();
    for (; it != itend; ++it)
    {
        if (!dynamic_cast<const llvm::ReturnInst*>(it->getTerminator()))
            continue;

        target.mergeGlobalLevel(mBlockOutputState.find(&*it)->second);
    }
}

bool
StackFrame::nextBlock()
{
    if (mCurrentState != mBlockOutputState[mCurrentBlock])
    {
        mChanged = true;
        mBlockOutputState[mCurrentBlock] = mCurrentState;
    }

    if (++mCurrentBlock == mFunction->end())
    {
        if (mChanged)
        {
            mChanged = false;
            startBlock(mFunction->begin());
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
        CANAL_ASSERT(*it != &block->getParent()->getEntryBlock() && "Entry block cannot have predecessors!");
        mBlockInputState[block].merge(mBlockOutputState[*it]);
    }

    mCurrentState = mBlockInputState[block];
    mCurrentInstruction = block->begin();
}

Stack::Stack(llvm::Module &module) : mModule(module), mHasEnteredNewFrame(false), mHasReturnedFromFrame(false)
{
}

bool
Stack::nextInstruction()
{
    if (mFrames.empty())
        return false;

    if (mHasReturnedFromFrame)
        mHasReturnedFromFrame = false;

    if (mHasEnteredNewFrame)
    {
        mHasEnteredNewFrame = false;
        return true;
    }

    StackFrame &currentFrame = mFrames.back();
    if (currentFrame.nextInstruction())
        return true;

    // End of function.
    mHasReturnedFromFrame = true;
    if (mFrames.size() == 1)
    {
        // End of program.  TODO: collect final values of global
        // variables and return value of main function.
        mFrames.pop_back();
        return false;
    }

    StackFrame &parentFrame = mFrames[mFrames.size() - 2];
    CANAL_ASSERT(llvm::isa<llvm::CallInst>(parentFrame.mCurrentInstruction) ||
                 llvm::isa<llvm::InvokeInst>(parentFrame.mCurrentInstruction));

    Value *returnedValue = currentFrame.getReturnedValue();
    if (returnedValue)
        parentFrame.mCurrentState.addFunctionVariable(*parentFrame.mCurrentInstruction, returnedValue);

    currentFrame.mergeGlobalVariables(parentFrame.mCurrentState);
    mFrames.pop_back();
    return false;
}

const llvm::Instruction &
Stack::getCurrentInstruction() const
{
    return *mFrames.back().mCurrentInstruction;
}

State &
Stack::getCurrentState()
{
    return mFrames.back().mCurrentState;
}

const llvm::Function &
Stack::getCurrentFunction() const
{
    return *mFrames.back().mFunction;
}

void
Stack::addFrame(const llvm::Function &function, const State &initialState)
{
    // Note that next instruction step is the jump to the first
    // instruction of the newly created frame.
    CANAL_ASSERT(!mHasEnteredNewFrame);
    if (mFrames.size() > 0)
        mHasEnteredNewFrame = true;

    mFrames.push_back(StackFrame(&function, initialState));
}

} // namespace Canal
