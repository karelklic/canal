#include "CommandShow.h"

CommandShow::CommandShow(Commands &commands)
    : Command("show",
              "Generic command for showing things about the interpreter",
              "",
              commands)
{
}

void
CommandShow::run(const std::vector<std::string> &args)
{
}
