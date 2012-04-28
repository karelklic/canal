#include "CommandStep.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include <cstdio>

CommandStep::CommandStep(Commands &commands)
    : Command("step",
              "s",
              "Step one instruction exactly",
              "Step one instruction exactly,",
              commands)
{
}

void
CommandStep::run(const std::vector<std::string> &args)
{
    if (!mCommands.mState || !mCommands.mState->isInterpreting())
    {
        puts("The program is not being interpreted.");
        return;
    }

    if (args.size() <= 1)
        mCommands.mState->step(1);
    else if (args.size() == 2)
    {
        bool success;
        int count = stringToPositiveInt(args[1].c_str(), success);
        if (success)
            mCommands.mState->step(count);
        else
            printf("Not a positive number: %s\n", args[1].c_str());
    }
    else
        puts("Invalid syntax.");
}
