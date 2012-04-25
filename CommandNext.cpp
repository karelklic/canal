#include "CommandNext.h"

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
}
