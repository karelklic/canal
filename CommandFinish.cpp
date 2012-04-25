#include "CommandFinish.h"

CommandFinish::CommandFinish(Commands &commands)
    : Command("finish",
              "Execute until selected stack frame returns",
              "",
              commands)
{
}

void
CommandFinish::run(const std::vector<std::string> &args)
{
}
