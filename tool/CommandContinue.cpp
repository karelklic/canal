#include "CommandContinue.h"
#include "Commands.h"
#include "State.h"

CommandContinue::CommandContinue(Commands &commands)
    : Command("continue",
              "",
              "Continue the program being interpreted",
              "",
              commands)
{
}

void
CommandContinue::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState() || !mCommands.getState()->isInterpreting())
    {
        llvm::outs() << "The program is not being interpreted.\n";
        return;
    }

    mCommands.getState()->run();
}
