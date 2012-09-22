#include "InterpreterBlockIterator.h"
#include "InterpreterBlockModule.h"
#include "InterpreterBlockFunction.h"
#include "InterpreterBlockBasicBlock.h"

namespace Canal {
namespace InterpreterBlock {

Iterator::Iterator(Module &module) : mModule(module)
{
    mFunction = module.begin();
    mBasicBlock = (*mFunction)->begin();
    mInstruction = (*mBasicBlock)->begin();
}

void
Iterator::nextInstruction()
{
}

void
Iterator::registerOnFixpointReached(void(*onFixpointReached)())
{
    mOnFixpointReached = onFixpointReached;
}

void
Iterator::registerOnFunctionEnter(void(*onFunctionEnter)())
{
    mOnFunctionEnter = onFunctionEnter;
}

void
Iterator::registerOnFunctionExit(void(*onFunctionExit)())
{
    mOnFunctionExit = onFunctionExit;
}

void
Iterator::registerOnBasicBlockEnter(void(*onBasicBlockEnter)())
{
    mOnBasicBlockEnter = onBasicBlockEnter;
}

void
Iterator::registerOnBasicBlockExit(void(*onBasicBlockExit)())
{
    mOnBasicBlockExit = onBasicBlockExit;
}

} // namespace InterpreterBlock
} // namespace Canal
