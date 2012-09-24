#include "InterpreterBlockIterator.h"
#include "InterpreterBlockModule.h"
#include "InterpreterBlockFunction.h"
#include "InterpreterBlockBasicBlock.h"
#include "InterpreterBlockIteratorCallback.h"
#include "Operations.h"

namespace Canal {
namespace InterpreterBlock {

static IteratorCallback emptyCallback;

Iterator::Iterator(Module &module, Operations &operations)
    : mModule(module),
      mOperations(operations),
      mChanged(true),
      mCallback(&emptyCallback)
{
    mFunction = --mModule.end();
    mBasicBlock = --(*mFunction)->end();
    mInstruction = --(*mBasicBlock)->end();
}

void
Iterator::nextInstruction()
{
    // 1. Call all onEnter callbacks.
    // 2. Move to next program instruction.
    // 3. Perform operation.
    // 4. Call all onExit callbacks.

    ++mInstruction;

    if (mInstruction == (*mBasicBlock)->end())
    {
        if (mState != (*mBasicBlock)->getOutputState())
        {
            (*mBasicBlock)->getOutputState().merge(mState);
            mChanged = true;
        }

        ++mBasicBlock;

        if (mBasicBlock == (*mFunction)->end())
        {
            ++mFunction;

            if (mFunction == mModule.end())
            {
                if (!mChanged)
                    mCallback->onFixpointReached();

                mChanged = false;
                mFunction = mModule.begin();
                mCallback->onModuleEnter();
            }

            mBasicBlock = (*mFunction)->begin();
            mCallback->onFunctionEnter(**mFunction);
        }

        // Update basic block input state from precedessors.  If it was
        // not changed, there is no point in interpreting the block's code
        // again.  The very first iteration is an exception, though.  The
        // code must be interpreted at least once.
        (*mFunction)->updateInputState(**mBasicBlock);

        mState = (*mBasicBlock)->getInputState();
        mInstruction = (*mBasicBlock)->begin();
        mCallback->onBasicBlockEnter(**mBasicBlock);
    }

    mCallback->onInstructionEnter(*mInstruction);
    mOperations.interpretInstruction(*mInstruction, mState);
    mCallback->onInstructionExit(*mInstruction);

    if (mInstruction == --(*mBasicBlock)->end())
    {
        mCallback->onBasicBlockExit(**mBasicBlock);

        if (mBasicBlock == --(*mFunction)->end())
        {
            (*mFunction)->updateOutputState();
            mCallback->onFunctionExit(**mFunction);

            if (mFunction == --mModule.end())
                mCallback->onModuleExit();
        }
    }
}

} // namespace InterpreterBlock
} // namespace Canal
