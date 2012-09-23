#ifndef LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H
#define LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H

#include "State.h"
#include <vector>
#include <llvm/BasicBlock.h>

namespace Canal {

class Operations;

namespace InterpreterBlock {

class Module;
class BasicBlock;
class Function;
class IteratorCallback;

/// Basic iterator that iterates over the whole program until a
/// fixpoint is reached.
class Iterator
{
protected:
    Module &mModule;
    Operations &mOperations;

    /// Indication of changed abstract state during last loop through
    /// the program.
    bool mChanged;

    std::vector<Function*>::const_iterator mFunction;
    std::vector<BasicBlock*>::const_iterator mBasicBlock;
    llvm::BasicBlock::const_iterator mInstruction;

    /// Current state.
    State mState;

    /// Callback functions.
    IteratorCallback *mCallback;

public:
    Iterator(Module &module,
             Operations &operations);

    /// One step of the interpreter.  Interprets current instruction
    /// and moves to the next one.
    void nextInstruction();

    void setCallback(IteratorCallback &callback) { mCallback = &callback; }

    const State &getCurrentState() const { return mState; }

    const Function &getCurrentFunction() const { return **mFunction; }

    const BasicBlock &getCurrentBasicBlock() const { return **mBasicBlock; }

    const llvm::Instruction &getCurrentInstruction() const { return *mInstruction; }
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H
