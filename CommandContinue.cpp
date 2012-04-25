#include "CommandContinue.h"

CommandContinue::CommandContinue(Commands &commands)
    : Command("continue",
              "Continue the program being interpreted",
              "",
              commands)
{
}

void
CommandContinue::run(const std::vector<std::string> &args)
{
}
