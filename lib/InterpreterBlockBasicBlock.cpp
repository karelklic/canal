#include "InterpreterBlockBasicBlock.h"
#include "Utils.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>

namespace Canal {
namespace InterpreterBlock {

BasicBlock::BasicBlock(const llvm::BasicBlock &basicBlock,
                       const Constructors &constructors)
    : mBasicBlock(basicBlock)
{
    // TODO: create bottom values for every instruction.  Both input
    // and output state.
    //CANAL_NOT_IMPLEMENTED();
}

} // namespace InterpreterBlock
} // namespace Canal
