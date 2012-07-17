#include "CommandStart.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include "../lib/Stack.h"
#include <cstdio>

CommandStart::CommandStart(Commands &commands)
    : Command("start",
              "",
              "Interpret the program until the beginning of the main procedure",
              "Interpret the program until the beginning of the main procedure.",
              commands)
{
}

void
CommandStart::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState())
    {
        puts("No program specified.  Use the \"file\" command.");
        return;
    }

    if (mCommands.getState()->isInterpreting())
    {
        puts("The program being interpreted has been started already.");
        return;
    }

    mCommands.getState()->addMainFrame();
    printf("Temporary breakpoint: \"main\".\n");
    print(mCommands.getState()->getStack().getCurrentInstruction());
}
