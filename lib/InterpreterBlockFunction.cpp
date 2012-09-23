#include "InterpreterBlockFunction.h"
#include "InterpreterBlockBasicBlock.h"
#include "Constructors.h"
#include <llvm/Function.h>
#include <llvm/Type.h>

namespace Canal {
namespace InterpreterBlock {

Function::Function(const llvm::Function &function,
                   const Constructors &constructors)
    : mFunction(function)
{
    // Initialize arguments.

    // Initialize basic blocks.
    {
        llvm::Function::const_iterator it = function.begin(),
            itend = function.end();
        for (; it != itend; ++it)
            mBasicBlocks.push_back(new BasicBlock(*it, constructors));
    }


    // Initialize return value.
    const llvm::Type *returnType = mFunction.getReturnType();
    if (returnType->isVoidTy())
        mReturnValue = NULL;
    else
        mReturnValue = constructors.create(*returnType);
}

Function::~Function()
{
    std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
    for (; it != mBasicBlocks.end(); ++it)
        delete *it;
}

const llvm::BasicBlock &
Function::getEntryBlock() const
{
    return mFunction.getEntryBlock();
}

llvm::StringRef
Function::getName() const
{
    return mFunction.getName();
}

} // namespace InterpreterBlock
} // namespace Canal

