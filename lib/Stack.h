#ifndef CANAL_STACK_H
#define CANAL_STACK_H

namespace Canal {

class StackFrame
{
public:
    StackFrame(const llvm::Function &function, const State &initialState);

    // Returns true if next instruction should be interpreted for this
    // frame, and false when fixpoint has been reached.
    bool nextInstruction();

    Value *getReturnValue() const;

public:
    const llvm::Function &mFunction;
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
    

public:
    std::vector<StackFrame> mFrames;
};

} // namespace Canal

#endif // CANAL_STACK_H
