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
}

class State
{
public:
    State(llvm::Module *module);
    ~State();

    void run();
    void step();
    void next();
    void finish();

    void addFunctionBreakpoint(const std::string &functionName);

public:
    llvm::Module *mModule;
    // Might be NULL if no module is loaded.
    Canal::Interpreter *mInterpreter;
    Canal::Stack *mStack;
    std::set<std::string> mFunctionBreakpoints;

protected:
    bool reachedBreakpoint();
};

#endif // CANAL_STATE_H
