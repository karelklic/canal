#ifndef LIBCANAL_INTERPRETER_BLOCK_H
#define LIBCANAL_INTERPRETER_BLOCK_H

#include "Constructors.h"
#include "Environment.h"
#include "Operations.h"
#include "InterpreterBlockFunction.h"
#include "InterpreterBlockIterator.h"

namespace Canal {

class SlotTracker;

namespace InterpreterBlock {

class Interpreter
{
public:
    /// @param module
    ///   Interpreter takes ownership of the module.
    Interpreter(const llvm::Module *module);

    const Environment &getEnvironment() const { return mEnvironment; }
    const llvm::Module &getModule() const { return mEnvironment.getModule(); }
    SlotTracker &getSlotTracker() const { return mEnvironment.getSlotTracker(); }
    const Operations &getOperations() const { return mOperations; }
    Iterator &getIterator() { return mIterator; }

protected:
    Environment mEnvironment;

    Operations mOperations;

    Constructors mConstructors;

    Iterator mIterator;

    // State of all functions is kept here.
    // Workers iterate on functions until the fixpoint is reached.
    std::vector<Function> mFunctions;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_H
