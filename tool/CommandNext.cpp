#include "CommandNext.h"
#include "Commands.h"
#include "State.h"
#include <cstdio>

CommandNext::CommandNext(Commands &commands)
    : Command("next",
              "Step one instruction on current level",
              "",
              commands)
{
}

void
CommandNext::run(const std::vector<std::string> &args)
{
    if (!mCommands.mState || !mCommands.mState->isInterpreting())
    {
        puts("The program is not being interpreted.");
        return;
    }

    mCommands.mState->next();
}
