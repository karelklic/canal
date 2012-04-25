#include "CommandQuit.h"

CommandQuit::CommandQuit(Commands &commands)
    : Command("quit",
              "Exit Canal",
              "",
              commands)
{
}

void
CommandQuit::run(const std::vector<std::string> &args)
{
}
