#ifndef LIBCANAL_INTERPRETER_BLOCK_BASIC_BLOCK_H
#define LIBCANAL_INTERPRETER_BLOCK_BASIC_BLOCK_H

namespace Canal {
namespace InterpreterBlock {

class BasicBlock
{
protected:
    const llvm::BasicBlock &mBasicBlock;

    State mInputState;
    State mOutputState;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_BASIC_BLOCK_H
