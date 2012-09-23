#ifndef LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H
#define LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H

#include "State.h"

namespace llvm {
class Function;
class BasicBlock;
class StringRef;
} // namespace llvm

namespace Canal {

class Domain;
class Constructors;

namespace InterpreterBlock {

class BasicBlock;

class Function
{
public:
    Function(const llvm::Function &function,
             const Constructors &constructors);

    virtual ~Function();

    const llvm::Function &getFunction() const { return mFunction; }

    const llvm::BasicBlock &getEntryBlock() const;

    BasicBlock &getBasicBlock(const llvm::BasicBlock &llvmBasicBlock);

    std::vector<BasicBlock*>::const_iterator begin() const { return mBasicBlocks.begin(); }
    std::vector<BasicBlock*>::const_iterator end() const { return mBasicBlocks.end(); }

    llvm::StringRef getName() const;

    /// Update basic block input state from its predecessors and
    /// function input state.
    /// @param basicBlock
    ///    Must be a member of this function.
    ///    Its input state is updated.
    void updateInputState(BasicBlock &basicBlock);

    /// Update function output state from basci block output states.
    void updateOutputState();

protected:
    const llvm::Function &mFunction;

    std::vector<BasicBlock*> mBasicBlocks;

    // Function arguments, global variables.
    State mInputState;

    // Returned value, global variables.
    State mOutputState;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H
