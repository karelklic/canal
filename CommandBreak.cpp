#include "CommandBreak.h"

CommandBreak::CommandBreak(Commands &commands)
    : Command("break",
              "Set breakpoint at specified line or function",
              "",
              commands)
{
}

void
CommandBreak::run(const std::vector<std::string> &args)
{
}
