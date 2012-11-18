#ifndef LIBCANAL_INTERPRETER_ITERATOR_H
#define LIBCANAL_INTERPRETER_ITERATOR_H

#include "State.h"
#include <vector>

namespace Canal {

class Operations;
class State;

namespace Widening {
class Manager;
} // namespace Widening

namespace Interpreter {

class Module;
class BasicBlock;
class Function;
class IteratorCallback;

/// Basic iterator that iterates over the whole program until a
/// fixpoint is reached.
class Iterator
{
    Module &mModule;
    Operations &mOperations;
    Widening::Manager &mWideningManager;

    /// Indication of changed abstract state during last loop through
    /// the program.
    bool mChanged;

    /// Indication that the iterator has been initialized and started
    /// iterating.
    bool mInitialized;

    /// Function of the instruction that will be interpreted in the
    /// next step.
    std::vector<Function*>::const_iterator mFunction;

    /// Basic block of the instruction that will be interpreted in the
    /// next step.
    std::vector<BasicBlock*>::const_iterator mBasicBlock;

    /// The instruction that will be interpreted in the next step.
    llvm::BasicBlock::const_iterator mInstruction;

    /// Current state.
    State *mState;

    /// Callback functions.
    IteratorCallback *mCallback;

public:
    Iterator(Module &module,
             Operations &operations,
             Widening::Manager &wideningManager);

    void initialize();

    /// One step of the interpreter.  Interprets the instruction
    /// and moves to the next one.
    void interpretInstruction();

    void setCallback(IteratorCallback &callback) { mCallback = &callback; }

    bool isInitialized() const { return mInitialized; }

    const State &getCurrentState() const { return *mState; }

    const Function &getCurrentFunction() const { return **mFunction; }

    const BasicBlock &getCurrentBasicBlock() const { return **mBasicBlock; }

    const llvm::Instruction &getCurrentInstruction() const { return *mInstruction; }

    std::string toString() const;

protected:
    void nextInstruction();
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_ITERATOR_H
