#ifndef LIBCANAL_INTERPRETER_BLOCK_H
#define LIBCANAL_INTERPRETER_BLOCK_H

#include "Constructors.h"
#include "Environment.h"
#include "Operations.h"
#include "InterpreterBlockModule.h"
#include "InterpreterBlockIterator.h"
#include "InterpreterBlockOperationsCallback.h"
#include <vector>

namespace Canal {

class SlotTracker;
class Domain;

namespace InterpreterBlock {

class Interpreter
{
public:
    /// @param module
    ///   Interpreter takes ownership of the module.
    Interpreter(const llvm::Module *module);
    virtual ~Interpreter();

    const Environment &getEnvironment() const { return mEnvironment; }
    SlotTracker &getSlotTracker() const { return mEnvironment.getSlotTracker(); }
    const Operations &getOperations() const { return mOperations; }
    Iterator &getIterator() { return mIterator; }

protected:
    Domain *onFunctionCall(const llvm::Function &function,
                           const std::vector<Domain*> &arguments);

protected:
    Environment mEnvironment;

    Constructors mConstructors;

    Module mModule;

    OperationsCallback mOperationsCallback;

    Operations mOperations;

    Iterator mIterator;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_H
