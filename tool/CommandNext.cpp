#include "CommandNext.h"
#include "Commands.h"
#include "State.h"

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
    mCommands.mState->next();
}
