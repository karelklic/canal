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

    std::vector<BasicBlock*>::const_iterator begin() const { return mBasicBlocks.begin(); }
    std::vector<BasicBlock*>::const_iterator end() const { return mBasicBlocks.end(); }

    llvm::StringRef getName() const;

protected:
    const llvm::Function &mFunction;

    std::vector<BasicBlock*> mBasicBlocks;

    State mArguments;

    Domain *mReturnValue;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H
