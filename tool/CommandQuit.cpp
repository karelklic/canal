#include "CommandQuit.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include <cstdlib>
#include <cstdio>

CommandQuit::CommandQuit(Commands &commands)
    : Command("quit",
              "Exit Canal",
              "",
              commands)
{
}

void
CommandQuit::run(const std::vector<std::string> &args)
{
    if (mCommands.mState && mCommands.mState->isInterpreting())
    {
        puts("A program is being interpreted.");
        bool agreed = askYesNo("Quit anyway?");
        if (!agreed)
        {
            puts("Not confirmed.");
            return;
        }
    }

    exit(0);
}
