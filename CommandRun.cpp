#include "CommandRun.h"
#include "Commands.h"
#include "State.h"
#include "lib/Utils.h"
#include "lib/State.h"
#include "lib/Stack.h"
#include "lib/Integer.h"
#include "lib/Pointer.h"
#include "lib/Array.h"
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <cstdio>

CommandRun::CommandRun(Commands &commands)
    : Command("run",
              "Start program interpretation",
              "",
              commands)
{
}

void
CommandRun::run(const std::vector<std::string> &args)
{
    const llvm::Function *function = mCommands.mState->mModule->getFunction("main");
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
            Canal::Pointer::InclusionBased *argv = new Canal::Pointer::InclusionBased();
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

    mCommands.mState->mStack->addFrame(*function, initialState);
    mCommands.mState->run();
}
