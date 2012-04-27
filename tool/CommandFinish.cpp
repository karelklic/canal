#include "CommandFinish.h"
#include "Commands.h"
#include "State.h"
#include <cstdio>

CommandFinish::CommandFinish(Commands &commands)
    : Command("finish",
              "Execute until selected stack frame returns",
              "",
              commands)
{
}

void
CommandFinish::run(const std::vector<std::string> &args)
{
    if (!mCommands.mState || !mCommands.mState->isInterpreting())
    {
        puts("The program is not being interpreted.");
        return;
    }

    mCommands.mState->finish();
}
