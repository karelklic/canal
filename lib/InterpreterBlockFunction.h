
namespace Canal {
namespace Interpreter {
namespace BasicBlock {

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

} // namespace BasicBlock
} // namespace Interpreter
} // namespace Canal
