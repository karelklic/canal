#ifndef LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H
#define LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H

#include "InterpreterBlockFunction.h"
#include <vector>

namespace Canal {
namespace InterpreterBlock {

/// Basic iterator that iterates over the whole program until a
/// fixpoint is reached.
class Iterator
{
public:
    /// Interprets next instruction.
    void nextInstruction();

    void registerOnFixpointReached(void(*fixpointReached)());
    void registerOnNextProgramIterationStarted(void(*nextProgramIterationStarted)());
    void registerOnFunctionEnter(void(*functionEnter)());
    void registerOnFunctionExit(void(*functionExit)());
    void registerOnBlockEnter(void(*blockEnter)());
    void registerOnBlockExit(void(*blockExit)());

protected:
    /// Indication of changed abstract state during last loop through
    /// the program.
    bool mChanged;

    std::vector<Function>::iterator mFunction;
    std::vector<BasicBlock>::iterator mBasicBlock;
    llvm::BasicBlock::const_iterator mInstruction;

    /// Current state.
    State mState;

    /// Callback functions.
    void(*mOnFixpointReached)();
    void(*mOnNextProgramIterationStarted)();
    void(*mOnFunctionEnter)();
    void(*mOnFunctionExit)();
    void(*mOnBlockEnter)();
    void(*mOnBlockExit)();
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H
