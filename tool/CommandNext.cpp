#include "CommandNext.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include <cstdio>

CommandNext::CommandNext(Commands &commands)
    : Command("next",
              "n",
              "Step one instruction on current level",
              "Step one instruction on current level.",
              commands)
{
}

void
CommandNext::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState() || !mCommands.getState()->isInterpreting())
    {
        puts("The program is not being interpreted.");
        return;
    }

    if (args.size() <= 1)
        mCommands.getState()->next(1);
    else if (args.size() == 2)
    {
        bool success;
        int count = stringToPositiveInt(args[1].c_str(), success);
        if (success)
            mCommands.getState()->next(count);
        else
            printf("Not a positive number: %s\n", args[1].c_str());
    }
    else
        puts("Invalid syntax.");
}
