#include "CommandPrint.h"

CommandPrint::CommandPrint(Commands &commands)
    : Command("print",
              "Print value of a variable",
              "",
              commands)
{
}

void
CommandPrint::run(const std::vector<std::string> &args)
{
}
