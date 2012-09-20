#ifndef LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H
#define LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H

namespace Canal {
namespace InterpreterBlock {

// Basic iterator that iterates over the whole program until a
// fixpoint is reached.
class Iterator
{
protected:
    bool mChanged;
    std::vector<Function>::iterator mFunction;
    std::vector<BasicBlock>::iterator mBasicBlock;
    llvm::BasicBlock::const_iterator mInstruction;
    State mState;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_ITERATOR_H
