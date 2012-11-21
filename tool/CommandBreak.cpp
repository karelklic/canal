#include "CommandBreak.h"
#include "Commands.h"
#include "State.h"

CommandBreak::CommandBreak(Commands &commands)
    : Command("break",
              "",
              "Set breakpoint at specified function",
              "Set breakpoint at specified function.",
              commands)
{
}

void
CommandBreak::run(const std::vector<std::string> &args)
{
    if (!mCommands.getState())
    {
        llvm::outs() << "No program specified.  Use the \"file\" command.\n";
        return;
    }

    if (args.size() == 1)
    {
        // TODO: Add breakpoint to current function.
        return;
    }

    std::vector<std::string>::const_iterator it = args.begin() + 1,
        itend = args.end();

    for (; it != itend; ++it)
        mCommands.getState()->addFunctionBreakpoint(*it);
}
