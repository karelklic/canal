#include "InterpreterBlockIterator.h"
#include "InterpreterBlockModule.h"
#include "InterpreterBlockFunction.h"
#include "InterpreterBlockBasicBlock.h"
#include "InterpreterBlockIteratorCallback.h"
#include "Operations.h"
#include "Environment.h"
#include "WideningManager.h"
#include "State.h"
#include <sstream>

namespace Canal {
namespace InterpreterBlock {

static IteratorCallback emptyCallback;

Iterator::Iterator(Module &module,
                   Operations &operations,
                   Widening::Manager &wideningManager)
    : mModule(module),
      mOperations(operations),
      mWideningManager(wideningManager),
      mChanged(true),
      mInitialized(false),
      mState(new State()),
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
    std::stringstream ss;
    ss << "***************************************" << std::endl;
    ss << "* iterator " << std::endl;
    ss << "***************************************" << std::endl;
    ss << "** function " << (*mFunction)->getName().str() << std::endl;
    ss << "*** basicBlock ";
    SlotTracker &slotTracker = mOperations.getEnvironment().getSlotTracker();
    const llvm::BasicBlock &basicBlock = (*mBasicBlock)->getLlvmBasicBlock();
    const llvm::Function &function = (*mFunction)->getLlvmFunction();
    slotTracker.setActiveFunction(function);
    if (basicBlock.hasName())
        ss << basicBlock.getName().str();
    else
        ss << "<label>:" << slotTracker.getLocalSlot(basicBlock);

    ss << std::endl;

    llvm::BasicBlock::const_iterator it = (*mBasicBlock)->begin();
    for (; it != mInstruction; ++it)
    {
        if (it->getType()->isVoidTy())
            continue;

        ss << mState->toString(*it, slotTracker);
    }

    return ss.str();
}

void
Iterator::nextInstruction()
{
    // Introduce the new instruction.
    ++mInstruction;

    if (mInstruction == (*mBasicBlock)->end())
    {
        mState->merge((*mBasicBlock)->getOutputState());
        if (*mState != (*mBasicBlock)->getOutputState())
        {
            mWideningManager.widen((*mBasicBlock)->getLlvmBasicBlock(),
                                   (*mBasicBlock)->getOutputState(),
                                   *mState);

            (*mBasicBlock)->getOutputState().merge(*mState);
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

        delete mState;
        mState = new State((*mBasicBlock)->getInputState());
        mInstruction = (*mBasicBlock)->begin();
        mCallback->onBasicBlockEnter(**mBasicBlock);
    }

    mCallback->onInstructionEnter(*mInstruction);
}

} // namespace InterpreterBlock
} // namespace Canal
