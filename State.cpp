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

void
State::run()
{
    bool finished = false;
    while (!finished)
    {
        finished = mInterpreter->step(*mStack);
        if (mStack->enteredNewFrame())
        {
            mStack->getCurrentFunction.getName();
        }
    }

void
State::step()
{
    mInterpreter->step(*mStack);
}

void
State::next()
{
    mInterpreter->step(*mStack);
}

void
State::finish()
{
    mInterpreter->step(*mStack);
}

void
State::continue_()
{
    mInterpreter->step(*mStack);
}

