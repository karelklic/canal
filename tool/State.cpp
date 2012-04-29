#include "State.h"
#include "Utils.h"
#include "../lib/Utils.h"
#include "../lib/State.h"
#include "../lib/Integer.h"
#include "../lib/Pointer.h"
#include "../lib/Array.h"
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/ADT/APInt.h>
#include <cstdio>

State::State(const llvm::Module *module) : mModule(module), mStack(*module), mSlotTracker(*module)
{
}

State::~State()
{
    delete mModule;
}

bool
State::isInterpreting() const
{
    return !mStack.getFrames().empty();
}

void
State::run()
{
    bool running = true;
    while (running)
    {
        running = mInterpreter.step(mStack);
        if (reachedBreakpoint())
            return;
    }

    puts("Program finished.");
}

void
State::step(int count)
{
    for (int i = 0; i < count; ++i)
    {
        bool running = mInterpreter.step(mStack);
        if (!running)
        {
            puts("Program finished.");
            return;
        }
        if (reachedBreakpoint())
            return;
    }

    print(mStack.getCurrentInstruction());
}

void
State::next(int count)
{
    for (int i = 0; i < count; ++i)
    {
        size_t stackSize = mStack.getFrames().size();
        bool running = mInterpreter.step(mStack);
        if (!running)
        {
            puts("Program finished.");
            return;
        }
        if (reachedBreakpoint())
            return;

        while (stackSize < mStack.getFrames().size())
        {
            mInterpreter.step(mStack);
            if (reachedBreakpoint())
                return;
        }
    }

    print(mStack.getCurrentInstruction());
}

void
State::finish()
{
    size_t stackSize = mStack.getFrames().size();
    while (stackSize <= mStack.getFrames().size())
    {
        bool running = mInterpreter.step(mStack);
        if (!running)
        {
            puts("Program finished.");
            return;
        }
        if (reachedBreakpoint())
            return;
    }
    print(mStack.getCurrentInstruction());
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

void
State::addMainFrame()
{
    const llvm::Function *function = mModule->getFunction("main");
    if (!function)
    {
        puts("Failed to find function \"main\".");
        return;
    }

    Canal::State initialState;
    llvm::Function::ArgumentListType::const_iterator it = function->getArgumentList().begin();
    for (int i = 0; it != function->getArgumentList().end(); ++it, ++i)
    {
        llvm::APInt signedOne(/*numBits=*/64, /*value=*/1, /*isSigned=*/true);
        switch (i)
        {
        case 0:
        {
            Canal::Value *argc = new Canal::Integer::Container(signedOne);
            initialState.addFunctionVariable(*it, argc);
            break;
        }
        case 1:
        {
            Canal::Pointer::InclusionBased *argv = new Canal::Pointer::InclusionBased(*mModule);
            initialState.addFunctionVariable(*it, argv);
            // Canal::Array::SingleItem *argumentList = new Canal::Array::SingleItem();
            // argumentList->mSize = new Canal::Integer::Container(signedOne);
            // argumentList->mValue = new Canal::Pointer::InclusionBased();
            // initialState.addGlobalBlock(/*instruction=*/(const llvm::Value&)1, argumentList);
            // argv->addMemoryTarget(/*instruction=*/*it, /*target=*/1, /*arrayOffset=*/NULL);

            // Canal::Array::SingleItem *programName = new Canal::Array::SingleItem();
            // llvm::APInt programNameSize(/*numBits=*/64, /*value=*/strlen("program") + 1, /*isSigned=*/true);
            // programName->mSize = new Canal::Integer::Container(/*number=*/programNameSize);
            // // Program name is an array of 8-bit integers.
            // programName->mValue = new Canal::Integer::Container(/*numBits=*/8);
            // intialState.addGlobalBlock(/*instruction=*/(const llvm::Value*)2, programName);
            // argumentList->mValue->addMemoryTarget(/*instruction=*/*it, /*target=*/2, /*arrayOffset=*/NULL);
            break;
        }
        default:
            CANAL_DIE();
        }
    }

    mStack.addFrame(*function, initialState);
}

bool
State::reachedBreakpoint()
{
    if (!mStack.hasEnteredNewFrame())
        return false;

    const std::string &name = mStack.getCurrentFunction().getName();
    std::set<std::string>::const_iterator it = mFunctionBreakpoints.find(name);
    if (it == mFunctionBreakpoints.end())
        return false;

    printf("Breakpoint reached: %s\n", name.c_str());
    print(mStack.getCurrentInstruction());
    return true;
}
