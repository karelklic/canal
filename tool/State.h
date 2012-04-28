#ifndef CANAL_STATE_H
#define CANAL_STATE_H

#include <set>
#include <string>

namespace llvm {
    class Module;
}

namespace Canal {
    class Interpreter;
    class Stack;
    class SlotTracker;
}

// State of the interpreter.
class State
{
public:
    State(llvm::Module *module);
    ~State();

    // Check if the interpreter is in the middle of interpretation.
    // This is true if something is on the stack.
    bool isInterpreting() const;

    void run();
    void step(int count);
    void next(int count);
    void finish();

    void addFunctionBreakpoint(const std::string &functionName);

    // Adds the "main" function to stack.
    void addMainFrame();

public:
    llvm::Module *mModule;
    // Might be NULL if no module is loaded.
    Canal::Interpreter *mInterpreter;
    Canal::Stack *mStack;
    std::set<std::string> mFunctionBreakpoints;
    Canal::SlotTracker *mSlotTracker;

protected:
    bool reachedBreakpoint();
};

#endif // CANAL_STATE_H
