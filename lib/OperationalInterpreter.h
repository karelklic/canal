#ifndef CANAL_OPERATIONAL_INTERPRETER_H
#define CANAL_OPERATIONAL_INTERPRETER_H

namespace llvm {
    class Function;
    class Module;
};

namespace Operational {

class State;
class Machine;

// Context-sensitive operational abstract interpreter.
//
// This class is not to be overloaded by custom abstract
// interpretation passes.  Overload Machine class to use desired
// abstract value classes.
class Interpreter
{
public:
    // @param machine
    //   Interpretation-specific abstract machine which interprets
    //   instructions on a machine state.
    // @param module
    //   LLVM module that contains all functions.
    Interpreter(Machine &machine, llvm::Module &module);

    // @param func
    //   Function to be interpreted. Its instructions will be applied in
    //   abstract domain to the provided state.
    void interpretFunction(const llvm::Function &func, const State &state);

protected:
    Machine &mMachine;
    llvm::Module &mModule;
};

} // namespace Operational

#endif
