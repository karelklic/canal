#include "CommandRun.h"
#include "Commands.h"
#include "State.h"
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
    mCommands.mState->addMainFrame();
    mCommands.mState->run();
}
