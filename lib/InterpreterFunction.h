#ifndef LIBCANAL_INTERPRETER_FUNCTION_H
#define LIBCANAL_INTERPRETER_FUNCTION_H

#include "State.h"

namespace Canal {

class Domain;
class Constructors;
class Environment;

namespace Interpreter {

class BasicBlock;

class Function
{
    const llvm::Function &mFunction;
    const Environment &mEnvironment;

    std::vector<BasicBlock*> mBasicBlocks;

    // Function arguments, global variables.
    State mInputState;

    // Returned value, global variables.
    State mOutputState;

public:
    Function(const llvm::Function &function,
             const Constructors &constructors);

    virtual ~Function();

    const llvm::Function &getLlvmFunction() const
    {
        return mFunction;
    }

    const llvm::BasicBlock &getLlvmEntryBlock() const;

    BasicBlock &getBasicBlock(const llvm::BasicBlock &llvmBasicBlock) const;

    std::vector<BasicBlock*>::const_iterator begin() const
    {
        return mBasicBlocks.begin();
    }

    std::vector<BasicBlock*>::const_iterator end() const
    {
        return mBasicBlocks.end();
    }

    bool empty() const
    {
        return mBasicBlocks.empty();
    }

    State &getInputState()
    {
        return mInputState;
    }

    const State &getInputState() const
    {
        return mInputState;
    }

    const State &getOutputState() const
    {
        return mOutputState;
    }

    llvm::StringRef getName() const;

    /// Update basic block input state from its predecessors and
    /// function input state.
    /// @param basicBlock
    ///    Must be a member of this function.
    ///    Its input state is updated.
    void initializeInputState(BasicBlock &basicBlock, State &state) const;

    /// Update function output state from basic block output states.
    void updateOutputState();

    /// Get memory usage (used byte count) of this function interpretation.
    size_t memoryUsage() const;

    std::string toString() const;

    void dumpToMetadata() const;
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_FUNCTION_H
