#include "InterpreterBlock.h"
#include "Utils.h"
#include "Pointer.h"
#include <llvm/Module.h>
#include <sstream>

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

std::string
Interpreter::toString() const
{
    std::stringstream ss;
    ss << mIterator.toString();
    ss << std::endl;
    ss << mModule.toString();
    return ss.str();
}

} // namespace InterpreterBlock
} // namespace Canal
