#ifndef LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H
#define LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H

#include "State.h"
#include <vector>
#include <llvm/BasicBlock.h>

namespace Canal {
namespace InterpreterBlock {

class Module;
class BasicBlock;
class Function;

/// Basic iterator that iterates over the whole program until a
/// fixpoint is reached.
class Iterator
{
public:
    Iterator(Module &module);

    /// One step of the interpreter.  Interprets current instruction
    /// and moves to the next one.
    void nextInstruction();

    void registerOnFixpointReached(void(*onFixpointReached)());
    void registerOnFunctionEnter(void(*onFunctionEnter)());
    void registerOnFunctionExit(void(*onFunctionExit)());
    void registerOnBasicBlockEnter(void(*onBasicBlockEnter)());
    void registerOnBasicBlockExit(void(*onBasicBlockExit)());

protected:
    Module &mModule;

    /// Indication of changed abstract state during last loop through
    /// the program.
    bool mChanged;

    std::vector<Function*>::const_iterator mFunction;
    std::vector<BasicBlock*>::const_iterator mBasicBlock;
    llvm::BasicBlock::const_iterator mInstruction;

    /// Current state.
    State mState;

    /// Callback functions.
    void(*mOnFixpointReached)();
    void(*mOnFunctionEnter)();
    void(*mOnFunctionExit)();
    void(*mOnBasicBlockEnter)();
    void(*mOnBasicBlockExit)();
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H
