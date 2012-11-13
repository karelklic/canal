#ifndef LIBCANAL_INTERPRETER_OPERATIONS_CALLBACK_H
#define LIBCANAL_INTERPRETER_OPERATIONS_CALLBACK_H

#include "OperationsCallback.h"

namespace Canal {

class Constructors;

namespace Interpreter {

extern bool printMissing;

class Module;

class OperationsCallback : public Canal::OperationsCallback
{
protected:
    Module &mModule;
    Constructors &mConstructors;

public:
    OperationsCallback(Module &module,
                       Constructors &mConstructors);

    virtual void onFunctionCall(const llvm::Function &function,
                                const State &callState,
                                State &resultState,
                                const llvm::Value &resultPlace);
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_OPERATIONS_CALLBACK_H
