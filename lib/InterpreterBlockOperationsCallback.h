#ifndef LIBCANAL_INTERPRETER_BLOCK_OPERATIONS_CALLBACK_H
#define LIBCANAL_INTERPRETER_BLOCK_OPERATIONS_CALLBACK_H

#include "OperationsCallback.h"

namespace Canal {
namespace InterpreterBlock {

class Module;

class OperationsCallback : public Canal::OperationsCallback
{
protected:
    Module &mModule;

public:
    OperationsCallback(Module &module);

    Domain *onFunctionCall(const llvm::Function &function,
                           const std::vector<Domain*> &arguments);
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_OPERATIONS_CALLBACK_H
