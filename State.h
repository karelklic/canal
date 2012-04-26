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

    llvm::Module *mModule;
    // Might be NULL if no module is loaded.
    Canal::Interpreter *mInterpreter;
    Canal::Stack *mStack;

    std::set<std::string> mFunctionNameBreakpoints;
};

#endif // CANAL_STATE_H
