#include "SlotTracker.h"
#include "../lib/Utils.h"
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Type.h>
#include <llvm/Instructions.h>
#include <llvm/ADT/SmallVector.h>

namespace Canal {

SlotTracker::SlotTracker(const llvm::Module &module)
    : mModule(module), mModuleProcessed(false),
      mFunction(NULL), mFunctionProcessed(false),
      mModuleNext(0), mFunctionNext(0), mMetadataNext(0)
{
}

void
SlotTracker::setActiveFunction(const llvm::Function &function)
{
    if (mFunction == &function)
        return;
    else if (mFunction)
    {
        // Discard the old function level map and list.
        mFunctionMap.clear();
        mFunctionList.clear();
    }

    mFunction = &function;
    mFunctionProcessed = false;
}

int
SlotTracker::getLocalSlot(const llvm::Value &value)
{
    CANAL_ASSERT_MSG(!llvm::isa<llvm::Constant>(value),
                     "Can't get a constant or global slot with this!");

    // Check for uninitialized state and do lazy initialization.
    initialize();

    // Find the value in the function-level map.
    ValueMap::iterator it = mFunctionMap.find(&value);
    return it == mFunctionMap.end() ? -1 : (int)it->second;
}

const llvm::Value *
SlotTracker::getLocalSlot(int num)
{
    // Check for uninitialized state and do lazy initialization.
    initialize();

    return mFunctionList.size() > num ? mFunctionList[num] : NULL;
}

int
SlotTracker::getGlobalSlot(const llvm::Value &value)
{
    // Check for uninitialized state and do lazy initialization.
    initialize();

    // Find the value in the module map.
    ValueMap::iterator it = mModuleMap.find(&value);
    return it == mModuleMap.end() ? -1 : (int)it->second;
}

const llvm::Value *
SlotTracker::getGlobalSlot(int num)
{
    // Check for uninitialized state and do lazy initialization.
    initialize();

    return mModuleList.size() > num ? mModuleList[num] : NULL;
}

int
SlotTracker::getMetadataSlot(const llvm::MDNode &node)
{
    // Check for uninitialized state and do lazy initialization.
    initialize();

    // Find the MDNode in the node map.
    mdn_iterator it = mMetadataMap.find(&node);
    return it == mMetadataMap.end() ? -1 : (int)it->second;
}

void
SlotTracker::initialize()
{
  if (!mModuleProcessed)
  {
      processModule();
      // Prevent re-processing next time we're called.
      mModuleProcessed = true;
  }

  if (mFunction && !mFunctionProcessed)
      processFunction();
}

void
SlotTracker::createFunctionSlot(const llvm::Value &value)
{
    CANAL_ASSERT_MSG(!value.getType()->isVoidTy() && !value.hasName(),
                     "Doesn't need a slot!");

    CANAL_ASSERT(mFunctionNext == mFunctionList.size());
    mFunctionList.push_back(&value);
    mFunctionMap[&value] = mFunctionNext++;
}

void
SlotTracker::createModuleSlot(const llvm::GlobalValue &value)
{
    CANAL_ASSERT_MSG(!value.getType()->isVoidTy(),
                     "Doesn't need a slot!");

    CANAL_ASSERT_MSG(!value.hasName(),
                     "Doesn't need a slot!");

    CANAL_ASSERT(mModuleNext == mModuleList.size());
    mModuleList.push_back(&value);
    mModuleMap[&value] = mModuleNext++;
}

void
SlotTracker::createMetadataSlot(const llvm::MDNode &node)
{
    // Don't insert if N is a function-local metadata, these are
    // always printed inline.
    if (!node.isFunctionLocal())
    {
        mdn_iterator it = mMetadataMap.find(&node);
        if (it != mMetadataMap.end())
            return;
        mMetadataMap[&node] = mMetadataNext++;
    }

    // Recursively add any MDNodes referenced by operands.
    for (unsigned i = 0, e = node.getNumOperands(); i != e; ++i)
    {
        if (const llvm::MDNode *operand = llvm::dyn_cast_or_null<llvm::MDNode>(node.getOperand(i)))
            createMetadataSlot(*operand);
    }
}

void
SlotTracker::processModule()
{
    // Add all of the unnamed global variables to the value table.
    for (llvm::Module::const_global_iterator I = mModule.global_begin(),
             E = mModule.global_end(); I != E; ++I)
    {
        if (!I->hasName())
            createModuleSlot(*I);
    }

    // Add metadata used by named metadata.
    for (llvm::Module::const_named_metadata_iterator
             I = mModule.named_metadata_begin(),
             E = mModule.named_metadata_end(); I != E; ++I)
    {
        const llvm::NamedMDNode *NMD = I;
        for (unsigned i = 0, e = NMD->getNumOperands(); i != e; ++i)
            createMetadataSlot(*NMD->getOperand(i));
    }

    // Add all the unnamed functions to the table.
    for (llvm::Module::const_iterator I = mModule.begin(), E = mModule.end();
         I != E; ++I)
    {
        if (!I->hasName())
            createModuleSlot(*I);
    }
}

void
SlotTracker::processFunction()
{
    mFunctionNext = 0;

    // Add all the function arguments with no names.
    for(llvm::Function::const_arg_iterator AI = mFunction->arg_begin(),
            AE = mFunction->arg_end(); AI != AE; ++AI)
    {
        if (!AI->hasName())
            createFunctionSlot(*AI);
    }

    llvm::SmallVector<std::pair<unsigned, llvm::MDNode*>, 4> MDForInst;

    // Add all of the basic blocks and instructions with no names.
    for (llvm::Function::const_iterator BB = mFunction->begin(),
             E = mFunction->end(); BB != E; ++BB)
    {
        if (!BB->hasName())
            createFunctionSlot(*BB);

        for (llvm::BasicBlock::const_iterator I = BB->begin(), E = BB->end(); I != E;
             ++I)
        {
            if (!I->getType()->isVoidTy() && !I->hasName())
                createFunctionSlot(*I);

            // Intrinsics can directly use metadata.  We allow direct calls to any
            // llvm.foo function here, because the target may not be linked into the
            // optimizer.
            if (const llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(I))
            {
                if (llvm::Function *F = CI->getCalledFunction())
                {
                    if (F->getName().startswith("llvm."))
                    {
                        for (unsigned i = 0, e = I->getNumOperands(); i != e; ++i)
                        {
                            if (llvm::MDNode *N = llvm::dyn_cast_or_null<llvm::MDNode>(I->getOperand(i)))
                                createMetadataSlot(*N);
                        }
                    }
                }
            }

            // Process metadata attached with this instruction.
            I->getAllMetadata(MDForInst);
            for (unsigned i = 0, e = MDForInst.size(); i != e; ++i)
                createMetadataSlot(*MDForInst[i].second);
            MDForInst.clear();
        }
    }

    mFunctionProcessed = true;
}

} // namespace Canal
