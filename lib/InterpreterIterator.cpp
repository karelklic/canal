#include "InterpreterIterator.h"
#include "InterpreterModule.h"
#include "InterpreterFunction.h"
#include "InterpreterBasicBlock.h"
#include "InterpreterIteratorCallback.h"
#include "Operations.h"
#include "Environment.h"
#include "WideningManager.h"
#include "MemoryState.h"
#include "Utils.h"

namespace Canal {
namespace Interpreter {

static IteratorCallback emptyCallback;

Iterator::Iterator(Module &module,
                   Operations &operations,
                   Widening::Manager &wideningManager)
    : mModule(module),
      mOperations(operations),
      mWideningManager(wideningManager),
      mChanged(true),
      mInitialized(false),
      mState(new Memory::State()),
      mCallback(&emptyCallback)
{
    if (!mModule.empty())
    {
        mFunction = --mModule.end();
        if (!(*mFunction)->empty())
        {
            mBasicBlock = --(*mFunction)->end();
            mInstruction = --(*mBasicBlock)->end();
        }
    }
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
    mOperations.interpretInstruction(*mInstruction, *mState);

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

std::string
Iterator::toString() const
{
    StringStream ss;
    ss << "***************************************\n";
    ss << "* iterator\n";
    ss << "***************************************\n";
    if (mInitialized)
    {
        ss << "** function " << (*mFunction)->getName() << "\n";
        ss << "*** basicBlock ";
        SlotTracker &slotTracker = mOperations.getEnvironment().getSlotTracker();
        const llvm::BasicBlock &basicBlock = (*mBasicBlock)->getLlvmBasicBlock();
        ss << Canal::getName(basicBlock, slotTracker) << "\n";

        llvm::BasicBlock::const_iterator it = (*mBasicBlock)->begin();
        for (; it != mInstruction; ++it)
        {
            if (it->getType()->isVoidTy())
                continue;

            ss << mState->toString(*it, slotTracker);
        }
    }
    else
        ss << "uninitialized\n";

    return ss.str();
}

void
Iterator::nextInstruction()
{
    // Introduce the new instruction.
    ++mInstruction;

    if (mInstruction == (*mBasicBlock)->end())
    {
        mState->join((*mBasicBlock)->getOutputState());
        if (*mState != (*mBasicBlock)->getOutputState())
        {
            mWideningManager.widen((*mBasicBlock)->getLlvmBasicBlock(),
                                   (*mBasicBlock)->getOutputState(),
                                   *mState);

            (*mBasicBlock)->getOutputState().join(*mState);
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

        delete mState;
        mState = new Memory::State((*mBasicBlock)->getInputState());
        (*mFunction)->initializeInputState(**mBasicBlock, *mState);
        mInstruction = (*mBasicBlock)->begin();
        mCallback->onBasicBlockEnter(**mBasicBlock);
    }

    mCallback->onInstructionEnter(*mInstruction);
}

} // namespace Interpreter
} // namespace Canal
