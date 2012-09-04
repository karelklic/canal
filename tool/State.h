#ifndef CANAL_STATE_H
#define CANAL_STATE_H

#include <set>
#include <string>
#include "lib/Environment.h"
#include "lib/Interpreter.h"
#include "lib/SlotTracker.h"
#include "lib/Stack.h"

namespace llvm {
    class Module;
}

// State of the interpreter.
class State
{
public:
    State(const llvm::Module *module);
    ~State();

    const llvm::Module &getModule() const { return *mModule; }
    const Canal::Environment &getEnvironment() const { return mEnvironment; }
    Canal::Stack &getStack() { return mStack; }
    const Canal::Stack &getStack() const { return mStack; }
    Canal::SlotTracker &getSlotTracker() { return mEnvironment.mSlotTracker; }

    // Check if the interpreter is in the middle of interpretation.
    // This is true if something is on the stack.
    bool isInterpreting() const;

    void run();
    void step(int count);
    void next(int count);
    void finish();

    // Add a breakpoint on function start.
    void addFunctionBreakpoint(const std::string &functionName);

    // Adds the "main" function to stack.
    void addMainFrame();

protected:
    bool reachedBreakpoint();

protected:
    const llvm::Module *mModule;
    Canal::Environment mEnvironment;
    Canal::Stack mStack;
    Canal::Interpreter mInterpreter;
    std::set<std::string> mFunctionBreakpoints;
};

#endif // CANAL_STATE_H
