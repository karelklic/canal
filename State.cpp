#include "State.h"
#include "lib/Interpreter.h"
#include "lib/Stack.h"
#include <llvm/Module.h>
#include <cstdio>

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
        if (reachedBreakpoint())
            return;
    }

    puts("Program finished.");
}

void
State::step()
{
    bool finished = mInterpreter->step(*mStack);
    if (finished)
        puts("Program finished.");
}

void
State::next()
{
    size_t stackSize = mStack->getFrames().size();
    bool finished = mInterpreter->step(*mStack);
    if (finished)
    {
        puts("Program finished.");
        return;
    }
    if (reachedBreakpoint())
        return;

    while (stackSize < mStack->getFrames().size())
    {
        mInterpreter->step(*mStack);
        if (reachedBreakpoint())
            return;
    }
}

void
State::finish()
{
    size_t stackSize = mStack->getFrames().size();
    while (stackSize <= mStack->getFrames().size())
    {
        bool finished = mInterpreter->step(*mStack);
        if (finished)
        {
            puts("Program finished.");
            return;
        }
        if (reachedBreakpoint())
            return;
    }
}

void
State::addFunctionBreakpoint(const std::string &functionName)
{
    if (!mModule->getFunction(functionName))
    {
        printf("Function \"%s\" not found.\n", functionName.c_str());
        return;
    }

    std::pair<std::set<std::string>::iterator, bool> result = mFunctionBreakpoints.insert(functionName);
    if (result.second)
        printf("New breakpoint on function \"%s\".\n", functionName.c_str());
    else
        printf("Breakpoint on function \"%s\" already exists.\n", functionName.c_str());
}

bool
State::reachedBreakpoint()
{
    if (!mStack->hasEnteredNewFrame())
        return false;

    const std::string &name = mStack->getCurrentFunction().getName();
    std::set<std::string>::const_iterator it = mFunctionBreakpoints.find(name);
    if (it == mFunctionBreakpoints.end())
        return false;

    printf("Breakpoint reached: %s\n", name.c_str());
    return true;
}
