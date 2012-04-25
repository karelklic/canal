#include "CommandRun.h"

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
}
