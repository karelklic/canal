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
class State;

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

    const Constructors &getConstructors() const { return mConstructors; }

    const Module &getModule() const { return mModule; }

    const Operations &getOperations() const { return mOperations; }

    Iterator &getIterator() { return mIterator; }

    const Iterator &getIterator() const { return mIterator; }

    const State &getCurrentState() const { return mIterator.getCurrentState(); }

    const Function &getCurrentFunction() const { return mIterator.getCurrentFunction(); }

    const BasicBlock &getCurrentBasicBlock() const { return mIterator.getCurrentBasicBlock(); }

    const llvm::Instruction &getCurrentInstruction() const { return mIterator.getCurrentInstruction(); }

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
