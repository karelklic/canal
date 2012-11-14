#ifndef LIBCANAL_INTERPRETER_BASIC_BLOCK_H
#define LIBCANAL_INTERPRETER_BASIC_BLOCK_H

#include "State.h"
#include <llvm/BasicBlock.h>

namespace Canal {

class Constructors;
class Environment;

namespace Interpreter {

class BasicBlock
{
protected:
    const llvm::BasicBlock &mBasicBlock;
    const Environment &mEnvironment;

    State mInputState;
    State mOutputState;

public:
    BasicBlock(const llvm::BasicBlock &basicBlock,
               const Constructors &constructors);

    const llvm::BasicBlock &getLlvmBasicBlock() const { return mBasicBlock; }

    llvm::BasicBlock::const_iterator begin() const { return mBasicBlock.begin(); }
    llvm::BasicBlock::const_iterator end() const { return mBasicBlock.end(); }

    State &getInputState() { return mInputState; }
    State &getOutputState() { return mOutputState; }

    std::string toString() const;
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BASIC_BLOCK_H
