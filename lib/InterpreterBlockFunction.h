#ifndef LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H
#define LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H

#include "InterpreterBlockBasicBlock.h"
#include "State.h"
#include <llvm/Function.h>

namespace Canal {

class Value;

namespace InterpreterBlock {

class Function
{
public:
    const llvm::BasicBlock &getEntryBlock() const;

protected:
    const llvm::Function &mFunction;
    std::vector<BasicBlock> mBasicBlocks;
    State mInput;
    Value *mReturnValue;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H
