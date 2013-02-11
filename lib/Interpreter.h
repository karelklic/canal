#ifndef LIBCANAL_INTERPRETER_H
#define LIBCANAL_INTERPRETER_H

#include "Constructors.h"
#include "Environment.h"
#include "Operations.h"
#include "InterpreterModule.h"
#include "InterpreterIterator.h"
#include "InterpreterOperationsCallback.h"
#include "WideningManager.h"

namespace Canal {
namespace Interpreter {

class Interpreter
{
    Environment mEnvironment;

    Constructors mConstructors;

    Module mModule;

    OperationsCallback mOperationsCallback;

    Operations mOperations;

    Widening::Manager mWideningManager;

    Iterator mIterator;

public:
    /// @param module
    ///   Interpreter takes ownership of the module.
    Interpreter(llvm::Module *module);
    virtual ~Interpreter();

    const Environment &getEnvironment() const
    {
        return mEnvironment;
    }

    SlotTracker &getSlotTracker() const
    {
        return mEnvironment.getSlotTracker();
    }

    const Constructors &getConstructors() const
    {
        return mConstructors;
    }

    const Module &getModule() const
    {
        return mModule;
    }

    const Operations &getOperations() const
    {
        return mOperations;
    }

    Iterator &getIterator()
    {
        return mIterator;
    }

    const Iterator &getIterator() const
    {
        return mIterator;
    }

    const Memory::State &getCurrentState() const
    {
        return mIterator.getCurrentState();
    }

    const Function &getCurrentFunction() const
    {
        return mIterator.getCurrentFunction();
    }

    const BasicBlock &getCurrentBasicBlock() const
    {
        return mIterator.getCurrentBasicBlock();
    }

    const llvm::Instruction &getCurrentInstruction() const
    {
        return mIterator.getCurrentInstruction();
    }

    std::string toString() const;
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_H
