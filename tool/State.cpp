#include "State.h"
#include "Utils.h"
#include "lib/Utils.h"
#include "lib/State.h"
#include "lib/ProductVector.h"
#include "lib/Pointer.h"
#include "lib/InterpreterFunction.h"

State::State(llvm::Module *module) : mInterpreter(module)
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
    llvm::outs() << "Entering function "
                 << mInterpreter.getCurrentFunction().getName() << ".\n"
                 << "Entering basic block.\n"
                 << mInterpreter.getCurrentInstruction() << "\n";
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

        if (mIteratorCallback.isFunctionEnter())
        {
            llvm::outs() << "Entering function "
                         << mInterpreter.getCurrentFunction().getName()
                         << ".\n";
        }

        if (mIteratorCallback.isBasicBlockEnter())
            llvm::outs() << "Entering basic block.\n";

        llvm::outs() << mInterpreter.getCurrentInstruction() << "\n";
    }
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

    llvm::outs() << "Entering function "
                 << mInterpreter.getCurrentFunction().getName() << ".\n"
                 << "Entering basic block.\n"
                 << mInterpreter.getCurrentInstruction() << "\n";
}

void
State::addFunctionBreakpoint(const std::string &functionName)
{
    if (!mInterpreter.getModule().getFunction(functionName))
    {
        llvm::outs() << "Function \"" << functionName << "\" not found.\n";
        return;
    }

    std::pair<std::set<std::string>::iterator, bool> result =
        mFunctionBreakpoints.insert(functionName);

    if (result.second)
    {
        llvm::outs() << "New breakpoint on function \"" << functionName
                     << "\".\n";
    }
    else
    {
        llvm::outs() << "Breakpoint on function \"" << functionName
                     << "\" already exists.\n";
    }
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

    llvm::outs() << "Entering function "
                 << mInterpreter.getCurrentFunction().getName() << ".\n"
                 << "Entering basic block.\n"
                 << "Breakpoint reached: " << name << "\n"
                 << mInterpreter.getCurrentInstruction() << "\n";

    return true;
}
