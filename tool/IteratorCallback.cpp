#include "IteratorCallback.h"
#include "lib/InterpreterBlockFunction.h"
#include <llvm/ADT/StringRef.h>
#include <cstdio>

void
IteratorCallback::onFixpointReached()
{
    puts("Program finished.");
    mFixpointReached = true;
}

void
IteratorCallback::onFunctionEnter(Canal::InterpreterBlock::Function &function)
{
    if (!mFixpointReached)
        mFunctionEnter = true;
}

void
IteratorCallback::onBasicBlockEnter(Canal::InterpreterBlock::BasicBlock &basicBlock)
{
    if (!mFixpointReached)
        mBasicBlockEnter = true;
}

void
IteratorCallback::onInstructionExit(const llvm::Instruction &instruction)
{
    mFunctionEnter = mBasicBlockEnter = false;
}
