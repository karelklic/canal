#include "State.h"
#include "lib/Interpreter.h"
#include "lib/Stack.h"
#include <llvm/Module.h>

State::State(llvm::Module *module)
{
    mModule = module;
    mInterpreter = new Canal::Interpreter();
    mStack = new Canal::Stack(*module);
}

State::~State()
{
    delete mInterpreter;
    delete mStack;
    delete mModule;
}
