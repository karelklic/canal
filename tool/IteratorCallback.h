#ifndef CANAL_ITERATOR_CALLBACK_H
#define CANAL_ITERATOR_CALLBACK_H

#include "lib/InterpreterBlockIteratorCallback.h"

class IteratorCallback : public Canal::InterpreterBlock::IteratorCallback
{
protected:
    bool mFixpointReached;

    bool mFunctionEnter;

    bool mBasicBlockEnter;

public:
    IteratorCallback() : mFixpointReached(false), mFunctionEnter(false),
        mBasicBlockEnter(false) {}

    virtual void onFixpointReached();

    virtual void onFunctionEnter(Canal::InterpreterBlock::Function &function);

    virtual void onBasicBlockEnter(Canal::InterpreterBlock::BasicBlock &basicBlock);

    virtual void onInstructionExit(const llvm::Instruction &instruction);

    bool isFixpointReached() const { return mFixpointReached; }

    bool isFunctionEnter() { return mFunctionEnter; }

    bool isBasicBlockEnter() { return mFunctionEnter; }
};

#endif // CANAL_ITERATOR_CALLBACK
