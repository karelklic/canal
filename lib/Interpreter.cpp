#include "Interpreter.h"
#include "Utils.h"
#include "Pointer.h"
#include <sstream>

namespace Canal {
namespace Interpreter {

Interpreter::Interpreter(llvm::Module *module)
    : mEnvironment(module),
      mConstructors(mEnvironment),
      mModule(*module, mConstructors),
      mOperationsCallback(mModule, mConstructors),
      mOperations(mEnvironment, mConstructors, mOperationsCallback),
      mIterator(mModule, mOperations, mWideningManager)
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

} // namespace Interpreter
} // namespace Canal
