#include "CommandContinue.h"
#include "Commands.h"
#include "State.h"
#include <cstdio>

CommandContinue::CommandContinue(Commands &commands)
    : Command("continue",
              "",
              "Continue the program being interpreted",
              "",
              commands)
{
}

void
CommandContinue::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState() || !mCommands.getState()->isInterpreting())
    {
        puts("The program is not being interpreted.");
        return;
    }

    mCommands.getState()->run();
}
