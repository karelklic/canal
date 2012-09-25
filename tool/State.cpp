#include "State.h"
#include "Utils.h"
#include "lib/Utils.h"
#include "lib/State.h"
#include "lib/IntegerContainer.h"
#include "lib/Pointer.h"
#include "lib/InterpreterBlockFunction.h"
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/ADT/APInt.h>
#include <cstdio>

State::State(const llvm::Module *module) : mInterpreter(module)
{
    mInterpreter.getIterator().setCallback(mIteratorCallback);
}

State::~State()
{
}

bool
State::isInterpreting() const
{
    return mInterpreter.getIterator().isInitialized() &&
        !mIteratorCallback.isFixpointReached();
}

void
State::start()
{
    mInterpreter.getIterator().initialize();
    print(mInterpreter.getCurrentInstruction());
}

void
State::run()
{
    mInterpreter.getIterator().initialize();

    while (!mIteratorCallback.isFixpointReached())
    {
        mInterpreter.getIterator().interpretInstruction();
        if (reachedBreakpoint())
            return;
    }
}

void
State::step(int count)
{
    for (int i = 0; i < count; ++i)
    {
        mInterpreter.getIterator().interpretInstruction();
        if (mIteratorCallback.isFixpointReached())
            return;
        if (reachedBreakpoint())
            return;
    }

    print(mInterpreter.getCurrentInstruction());
}

void
State::finish()
{
    while (!mIteratorCallback.isFunctionEnter())
    {
        mInterpreter.getIterator().interpretInstruction();
        if (mIteratorCallback.isFixpointReached())
            return;
        if (reachedBreakpoint())
            return;
    }

    print(mInterpreter.getCurrentInstruction());
}

void
State::addFunctionBreakpoint(const std::string &functionName)
{
    if (!mInterpreter.getModule().getFunction(functionName))
    {
        printf("Function \"%s\" not found.\n", functionName.c_str());
        return;
    }

    std::pair<std::set<std::string>::iterator, bool> result =
        mFunctionBreakpoints.insert(functionName);

    if (result.second)
        printf("New breakpoint on function \"%s\".\n", functionName.c_str());
    else
        printf("Breakpoint on function \"%s\" already exists.\n", functionName.c_str());
}

bool
State::reachedBreakpoint()
{
    if (!mIteratorCallback.isFunctionEnter())
        return false;

    llvm::StringRef name = mInterpreter.getCurrentFunction().getName();
    std::set<std::string>::const_iterator it = mFunctionBreakpoints.find(name);
    if (it == mFunctionBreakpoints.end())
        return false;

    printf("Breakpoint reached: %s\n", name.str().c_str());
    print(mInterpreter.getCurrentInstruction());
    return true;
}
