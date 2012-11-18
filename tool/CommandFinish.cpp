#include "CommandFinish.h"
#include "Commands.h"
#include "State.h"

CommandFinish::CommandFinish(Commands &commands)
    : Command("finish",
              "fin",
              "Execute until selected stack frame returns",
              "Execute until selected stack frame returns.",
              commands)
{
}

void
CommandFinish::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState() || !mCommands.getState()->isInterpreting())
    {
        llvm::outs() << "The program is not being interpreted.\n";
        return;
    }

    mCommands.getState()->finish();
}
