#include "CommandStart.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"

CommandStart::CommandStart(Commands &commands)
    : Command("start",
              "",
              "Interpret the program until the beginning of the main procedure",
              "Interpret the program until the beginning of the main procedure.",
              commands)
{
}

void
CommandStart::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState())
    {
        llvm::outs() << "No program specified.  Use the \"file\" command.\n";
        return;
    }

    if (mCommands.getState()->isInterpreting())
    {
        llvm::outs() << "The program being interpreted has been started already.\n";
        return;
    }

    mCommands.getState()->start();
}
