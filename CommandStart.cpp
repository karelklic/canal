#include "CommandStart.h"
#include "Commands.h"
#include "State.h"
#include <cstdio>

CommandStart::CommandStart(Commands &commands)
    : Command("start",
              "Interpret the program until the beginning of the main procedure",
              "",
              commands)
{
}

void
CommandStart::run(const std::vector<std::string> &args)
{
    mCommands.mState->addMainFrame();
    printf("Main reached.\n");
}
