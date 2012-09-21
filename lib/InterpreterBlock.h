#ifndef LIBCANAL_INTERPRETER_BLOCK_H
#define LIBCANAL_INTERPRETER_BLOCK_H

#include "Environment.h"
#include "Interpreter.h"
#include "InterpreterBlockFunction.h"
#include "InterpreterBlockIterator.h"

namespace Canal {

class SlotTracker;

namespace InterpreterBlock {

class Interpreter
{
public:
    Interpreter(const llvm::Module *module);

    const Environment &getEnvironment() const { return mEnvironment; }
    const llvm::Module &getModule() const { return mEnvironment.getModule(); }
    SlotTracker &getSlotTracker() const { return mEnvironment.getSlotTracker(); }
    const Operations &getOperations() const { return mOperations; }
    Iterator &getIterator() { return mIterator; }

protected:
    Environment mEnvironment;

    Operations mOperations;

    Iterator mIterator;

    // State of all functions is kept here.
    // Workers iterate on functions until the fixpoint is reached.
    std::vector<Function> mFunctions;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_H
