#include "InterpreterBlockFunction.h"
#include "InterpreterBlockBasicBlock.h"
#include "Constructors.h"
#include "Environment.h"
#include "Domain.h"
#include "Utils.h"
#include <llvm/Function.h>
#include <llvm/Type.h>
#include <llvm/Support/CFG.h>
#include <llvm/Instructions.h>
#include <sstream>

namespace Canal {
namespace InterpreterBlock {

Function::Function(const llvm::Function &function,
                   const Constructors &constructors)
    : mFunction(function), mEnvironment(constructors.getEnvironment())
{
    // Initialize input state.
    {
        llvm::Function::const_arg_iterator it = function.arg_begin(),
            itend = function.arg_end();

        for (; it != itend; ++it)
        {
            Domain *argument = constructors.create(*it->getType());
            mInputState.addFunctionVariable(*it, argument);
        }
    }

    // Initialize basic blocks.
    {
        llvm::Function::const_iterator it = function.begin(),
            itend = function.end();

        for (; it != itend; ++it)
            mBasicBlocks.push_back(new BasicBlock(*it, constructors));
    }

    // Initialize output state.
    const llvm::Type *returnType = mFunction.getReturnType();
    if (!returnType->isVoidTy())
        mOutputState.mReturnedValue = constructors.create(*returnType);
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

BasicBlock &
Function::getBasicBlock(const llvm::BasicBlock &llvmBasicBlock)
{
    std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
    for (; it != mBasicBlocks.end(); ++it)
    {
        if (&(*it)->getBasicBlock() == &llvmBasicBlock)
            return **it;
    }

    CANAL_FATAL_ERROR("Failed to find certain basic block.");
}

llvm::StringRef
Function::getName() const
{
    return mFunction.getName();
}

void
Function::updateBasicBlockInputState(BasicBlock &basicBlock)
{
    const llvm::BasicBlock &llvmBasicBlock = basicBlock.getBasicBlock();

    // Merge out states of predecessors to input state of
    // current block.
    llvm::const_pred_iterator it = llvm::pred_begin(&llvmBasicBlock),
        itend = llvm::pred_end(&llvmBasicBlock);

    for (; it != itend; ++it)
    {
        BasicBlock &predBlock = getBasicBlock(**it);
        basicBlock.getInputState().merge(predBlock.getOutputState());
    }

    if (&llvmBasicBlock == &getEntryBlock())
        basicBlock.getInputState().merge(mInputState);
}

void
Function::updateOutputState()
{
    std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
    for (; it != mBasicBlocks.end(); ++it)
    {
        if (!llvm::isa<llvm::ReturnInst>((*it)->getBasicBlock().getTerminator()))
            continue;

        // Merge global blocks, global variables.  Merge function
        // blocks that do not belong to this function.  Merge returned
        // value.
        mOutputState.mergeGlobal((*it)->getOutputState());
        mOutputState.mergeReturnedValue((*it)->getOutputState());
        mOutputState.mergeForeignFunctionBlocks((*it)->getOutputState(),
                                                mFunction);
    }
}

std::string
Function::toString() const
{
    std::stringstream ss;

    ss << "*******************************************" << std::endl;
    ss << "** function " << mFunction.getName().str() << std::endl;
    ss << "*******************************************" << std::endl;
    ss << std::endl;

    // Print function arguments.
    {
        SlotTracker &slotTracker = mEnvironment.getSlotTracker();
        llvm::Function::ArgumentListType::const_iterator it =
            mFunction.getArgumentList().begin(),
            itend = mFunction.getArgumentList().end();

        for (; it != itend; ++it)
            ss << mInputState.toString(*it, slotTracker);

        if (mFunction.getArgumentList().begin() != itend)
            ss << std::endl;
    }

    // Print function result.
    if (!mFunction.getReturnType()->isVoidTy())
    {
        ss << "returnedValue = ";
        if (mOutputState.mReturnedValue)
        {
            ss << Canal::indentExceptFirstLine(
                mOutputState.mReturnedValue->toString(),
                16);
        }
        else
            ss << "undefined" << std::endl;

        ss << std::endl;
    }

    // Print basic blocks.
    {
        std::vector<BasicBlock*>::const_iterator it = mBasicBlocks.begin();
        for (; it != mBasicBlocks.end(); ++it)
            ss << (*it)->toString();
    }

    return ss.str();
}

} // namespace InterpreterBlock
} // namespace Canal

