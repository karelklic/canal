#ifndef CANAL_STATE_H
#define CANAL_STATE_H

#include "IteratorCallback.h"
#include "lib/Interpreter.h"
#include <set>
#include <string>

// State of the interpreter.
class State
{
    Canal::Interpreter::Interpreter mInterpreter;

    std::set<std::string> mFunctionBreakpoints;

    IteratorCallback mIteratorCallback;

public:
    State(llvm::Module *module);
    ~State();

    Canal::Interpreter::Interpreter &getInterpreter()
    {
        return mInterpreter;
    }

    const Canal::Interpreter::Interpreter &getInterpreter() const
    {
        return mInterpreter;
    }

    const Canal::Environment &getEnvironment() const
    {
        return mInterpreter.getEnvironment();
    }

    const llvm::Module &getModule() const
    {
        return getEnvironment().getModule();
    }

    llvm::Module &getModule()
    {
        return getEnvironment().getModule();
    }

    Canal::SlotTracker &getSlotTracker() const
    {
        return getEnvironment().getSlotTracker();
    }

    const IteratorCallback &getIteratorCallback() const
    {
        return mIteratorCallback;
    }

    // Check if the interpreter is in the middle of interpretation.
    // This is true if something is on the stack.
    bool isInterpreting() const;

    void start();
    void run();
    void step(int count);
    void finish();

    // Add a breakpoint on function start.
    void addFunctionBreakpoint(const std::string &functionName);

protected:
    bool reachedBreakpoint();
};

#endif // CANAL_STATE_H
