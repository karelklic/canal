#include "CommandStart.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include "../lib/Stack.h"
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
    printf("Temporary breakpoint: \"main\".\n");
    print(mCommands.mState->mStack->getCurrentInstruction());
}
