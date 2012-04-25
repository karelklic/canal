#include "CommandStep.h"

CommandStep::CommandStep(Commands &commands)
    : Command("step",
              "Step one instruction exactly",
              "",
              commands)
{
}

void
CommandStep::run(const std::vector<std::string> &args)
{
}
