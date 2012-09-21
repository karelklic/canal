#include "InterpreterBlockIterator.h"

namespace Canal {
namespace InterpreterBlock {

void
Iterator::registerOnFixpointReached(void(*fixpointReached)())
{
    mOnFixpointReached = fixpointReached;
}

void
Iterator::registerOnNextProgramIterationStarted(void(*nextProgramIterationStarted)())
{
    mOnNextProgramIterationStarted = nextProgramIterationStarted;
}

void
Iterator::registerOnFunctionEnter(void(*functionEnter)())
{
    mOnFunctionEnter = functionEnter;
}

void
Iterator::registerOnFunctionExit(void(*functionExit)())
{
    mOnFunctionExit = functionExit;
}

void
Iterator::registerOnBlockEnter(void(*blockEnter)())
{
    mOnBlockEnter = blockEnter;
}

void
Iterator::registerOnBlockExit(void(*blockExit)())
{
    mOnBlockExit = blockExit;
}

} // namespace InterpreterBlock
} // namespace Canal
