#include "Interpreter.h"
#include "Utils.h"

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
    StringStream ss;
    ss << mIterator.toString();
    ss << "\n";
    ss << mModule.toString();
    return ss.str();
}

} // namespace Interpreter
} // namespace Canal
