#ifndef CANAL_STATE_H
#define CANAL_STATE_H

#include <set>
#include <string>
#include "lib/InterpreterBlock.h"

namespace llvm {
class Module;
} // namespace llvm

// State of the interpreter.
class State
{
public:
    State(const llvm::Module *module);
    ~State();

    const Canal::Environment &getEnvironment() const { return mInterpreter.getEnvironment(); }
    const llvm::Module &getModule() const { return getEnvironment().getModule(); }
    Canal::SlotTracker &getSlotTracker() const { return getEnvironment().getSlotTracker(); }

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
    Canal::InterpreterBlock::Interpreter mInterpreter;
    std::set<std::string> mFunctionBreakpoints;
};

#endif // CANAL_STATE_H
