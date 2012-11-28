#ifndef CANAL_ITERATOR_CALLBACK_H
#define CANAL_ITERATOR_CALLBACK_H

#include "Prereq.h"
#include "lib/InterpreterIteratorCallback.h"

class IteratorCallback : public Canal::Interpreter::IteratorCallback
{
protected:
    bool mFixpointReached;

    bool mFunctionEnter;

    bool mBasicBlockEnter;

public:
    IteratorCallback()
        : mFixpointReached(false),
          mFunctionEnter(false),
          mBasicBlockEnter(false)
    {
    }

    virtual void onFixpointReached();

    virtual void onFunctionEnter(Canal::Interpreter::Function &function);

    virtual void onBasicBlockEnter(Canal::Interpreter::BasicBlock &basicBlock);

    virtual void onInstructionExit(const llvm::Instruction &instruction);

    bool isFixpointReached() const
    {
        return mFixpointReached;
    }

    bool isFunctionEnter()
    {
        return mFunctionEnter;
    }

    bool isBasicBlockEnter()
    {
        return mFunctionEnter;
    }
};

#endif // CANAL_ITERATOR_CALLBACK
