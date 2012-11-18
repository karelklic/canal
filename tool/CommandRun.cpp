#include "CommandRun.h"
#include "Commands.h"
#include "State.h"

CommandRun::CommandRun(Commands &commands)
    : Command("run",
              "",
              "Start program interpretation",
              "Start program interpretation.",
              commands)
{
}

void
CommandRun::run(const std::vector<std::string> &args)
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

    mCommands.getState()->run();
}
