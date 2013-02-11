#ifndef LIBCANAL_INTERPRETER_OPERATIONS_CALLBACK_H
#define LIBCANAL_INTERPRETER_OPERATIONS_CALLBACK_H

#include "OperationsCallback.h"

namespace Canal {
namespace Interpreter {

extern bool printMissing;

class Module;

class OperationsCallback : public Canal::OperationsCallback
{
    Module &mModule;
    Constructors &mConstructors;

public:
    OperationsCallback(Module &module,
                       Constructors &mConstructors);

    virtual void onFunctionCall(const llvm::Function &function,
                                const Memory::State &callState,
                                Memory::State &resultState,
                                const llvm::Value &resultPlace);

    // char *strcat(char *destination, const char *source);
    void onFunctionCallStrcat(const llvm::Function &function,
                              const Memory::State &callState,
                              Memory::State &resultState,
                              const llvm::Value &resultPlace);
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_OPERATIONS_CALLBACK_H
