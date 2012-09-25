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
    {
        std::printf("Entering function %s.\n", function.getName().str().c_str());
        mFunctionEnter = true;
    }
}

void
IteratorCallback::onBasicBlockEnter(Canal::InterpreterBlock::BasicBlock &basicBlock)
{
    if (!mFixpointReached)
        std::printf("Entering basic block.\n");
}

void
IteratorCallback::onInstructionExit(const llvm::Instruction &instruction)
{
    mFunctionEnter = false;
}
