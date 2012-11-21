#include "CommandStep.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"

CommandStep::CommandStep(Commands &commands)
    : Command("step",
              "s",
              "Step one instruction exactly",
              "Step one instruction exactly,",
              commands)
{
}

void
CommandStep::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState() || !mCommands.getState()->isInterpreting())
    {
        llvm::outs() << "The program is not being interpreted.\n";
        return;
    }

    if (args.size() <= 1)
        mCommands.getState()->step(1);
    else if (args.size() == 2)
    {
        bool success;
        int count = stringToPositiveInt(args[1].c_str(), success);
        if (success)
            mCommands.getState()->step(count);
        else
            llvm::outs() << "Not a positive number: " << args[1] << "\n";
    }
    else
        llvm::outs() << "Invalid syntax.\n";
}
