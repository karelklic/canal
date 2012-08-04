#ifndef LIBCANAL_STACK_H
#define LIBCANAL_STACK_H

#include <map>
#include <vector>
#include <llvm/Function.h>
#include "State.h"

namespace Canal {

class Value;

class StackFrame
{
public:
    StackFrame(const llvm::Function *function,
               const State &initialState);

    // Returns true if next instruction should be interpreted for this
    // frame, and false when fixpoint has been reached.
    bool nextInstruction();

    Value *getReturnedValue() const;
    void mergeGlobalVariables(State &target) const;

public:
    // Never is NULL.  It is a pointer just to allow storing instances
    // of this class in std::vector.
    const llvm::Function *mFunction;
    std::map<const llvm::BasicBlock*, State> mBlockInputState;
    std::map<const llvm::BasicBlock*, State> mBlockOutputState;
    llvm::Function::const_iterator mCurrentBlock;
    State mCurrentState;
    llvm::BasicBlock::const_iterator mCurrentInstruction;
    bool mChanged;

private:
    bool nextBlock();
    void startBlock(llvm::Function::const_iterator block);
};

class Stack
{
public:
    Stack();

    bool nextInstruction();
    bool hasEnteredNewFrame() const { return mHasEnteredNewFrame; }
    bool hasReturnedFromFrame() const { return mHasReturnedFromFrame; }
    std::vector<StackFrame> &getFrames() { return mFrames; }
    const std::vector<StackFrame> &getFrames() const { return mFrames; }
    const llvm::Instruction &getCurrentInstruction() const;
    State &getCurrentState();
    const llvm::Function &getCurrentFunction() const;

    // @param function
    //   Function to be interpreted. Its instructions will be applied
    //   in abstract domain on the provided input state.
    // @param initialState
    //   Initial state when entering the function.  It includes global
    //   variables and function arguments.
    void addFrame(const llvm::Function &function,
                  const State &initialState);

protected:
    std::vector<StackFrame> mFrames;
    bool mHasEnteredNewFrame;
    bool mHasReturnedFromFrame;
};

} // namespace Canal

#endif // LIBCANAL_STACK_H
