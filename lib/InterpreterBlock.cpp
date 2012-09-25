#include "InterpreterBlock.h"
#include "Utils.h"
#include "Pointer.h"
#include <llvm/Module.h>

namespace Canal {
namespace InterpreterBlock {

Interpreter::Interpreter(const llvm::Module *module)
    : mEnvironment(module),
      mConstructors(mEnvironment),
      mModule(*module, mConstructors),
      mOperationsCallback(mModule, mConstructors),
      mOperations(mEnvironment, mConstructors, mOperationsCallback),
      mIterator(mModule, mOperations)
{
}

Interpreter::~Interpreter()
{
}

} // namespace InterpreterBlock
} // namespace Canal
