#include "CommandStep.h"
#include "Commands.h"
#include "State.h"
#include <cstdio>

CommandStep::CommandStep(Commands &commands)
    : Command("step",
              "Step one instruction exactly",
              "",
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

    mCommands.mState->step();
}
