

BasicBlock::BasicBlock(const llvm::BasicBlock &basicBlock)
    : mBasicBlock(basicBlock)
{
    // TODO: create bottom values for every instruction.
}

void
BasicBlock::mergeOutputToInput(const BasicBlock &basicBlock)
{
    CANAL_ASSERT_MSG(&mBasicBlock != mBasicBlock.getParent()->getEntryBlock(),
                     "Entry block cannot have predecessors!");

    mInputState.merge(basicBlock.mOutputState);
}
