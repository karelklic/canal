#include "Interpreter.h"
#include "Utils.h"
#include "Pointer.h"

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

void
Interpreter::dumpToMetadata () const
{
    return mModule.dumpToMetadata();
}

} // namespace Interpreter
} // namespace Canal
