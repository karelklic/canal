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
        : mModule(module), mOperations(operations), mCallback(&emptyCallback)
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
        ++mBasicBlock;

        if (mBasicBlock == (*mFunction)->end())
        {
            ++mFunction;

            if (mFunction == mModule.end())
            {
                if (!mChanged)
                    mCallback->onFixpointReached();

                mFunction = mModule.begin();
                mCallback->onModuleEnter();
            }

            mBasicBlock = (*mFunction)->begin();
            mCallback->onFunctionEnter(**mFunction);
        }

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
            mCallback->onFunctionExit(**mFunction);

            if (mFunction == --mModule.end())
                mCallback->onModuleExit();
        }
    }
}

} // namespace InterpreterBlock
} // namespace Canal
