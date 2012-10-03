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
      mInitialized(false),
      mCallback(&emptyCallback)
{
    mFunction = --mModule.end();
    mBasicBlock = --(*mFunction)->end();
    mInstruction = --(*mBasicBlock)->end();
}

void
Iterator::initialize()
{
    mInitialized = true;
    nextInstruction();
}

void
Iterator::interpretInstruction()
{
    // Interpret the instruction.
    mOperations.interpretInstruction(*mInstruction, mState);

    // Leave the instruction.
    mCallback->onInstructionExit(*mInstruction);

    if (mInstruction == --(*mBasicBlock)->end())
    {
        mCallback->onBasicBlockExit(**mBasicBlock);

        if (mBasicBlock == --(*mFunction)->end())
        {
            (*mFunction)->updateOutputState();
            mCallback->onFunctionExit(**mFunction);

            if (mFunction == --mModule.end())
            {
                mModule.updateGlobalState();
                mCallback->onModuleExit();
            }
        }
    }

    // Move to the next instruction.
    nextInstruction();
}

void
Iterator::nextInstruction()
{
    // Introduce the new instruction.
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
        (*mFunction)->updateBasicBlockInputState(**mBasicBlock);

        mState = (*mBasicBlock)->getInputState();
        mInstruction = (*mBasicBlock)->begin();
        mCallback->onBasicBlockEnter(**mBasicBlock);
    }

    mCallback->onInstructionEnter(*mInstruction);
}

} // namespace InterpreterBlock
} // namespace Canal
