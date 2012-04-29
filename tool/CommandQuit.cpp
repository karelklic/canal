#include "CommandQuit.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include <cstdlib>
#include <cstdio>

CommandQuit::CommandQuit(Commands &commands)
    : Command("quit",
              "",
              "Exit Canal",
              "Exit Canal.  If a program is being interpreted, it asks for confirmation before terminating the interpretation.",
              commands)
{
}

void
CommandQuit::run(const std::vector<std::string> &args)
{
    if (mCommands.getState() && mCommands.getState()->isInterpreting())
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
