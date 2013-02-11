#ifndef LIBCANAL_INTERPRETER_BASIC_BLOCK_H
#define LIBCANAL_INTERPRETER_BASIC_BLOCK_H

#include "MemoryState.h"

namespace Canal {
namespace Interpreter {

class BasicBlock
{
    const llvm::BasicBlock &mBasicBlock;
    const Environment &mEnvironment;

    Memory::State mInputState;
    Memory::State mOutputState;

public:
    BasicBlock(const llvm::BasicBlock &basicBlock,
               const Constructors &constructors);

    const llvm::BasicBlock &getLlvmBasicBlock() const
    {
        return mBasicBlock;
    }

    llvm::BasicBlock::const_iterator begin() const
    {
        return mBasicBlock.begin();
    }

    llvm::BasicBlock::const_iterator end() const
    {
        return mBasicBlock.end();
    }

    Memory::State &getInputState()
    {
        return mInputState;
    }

    Memory::State &getOutputState()
    {
        return mOutputState;
    }

    /// Get memory usage (used byte count) of this basic block interpretation.
    size_t memoryUsage() const;

    std::string toString() const;
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BASIC_BLOCK_H
