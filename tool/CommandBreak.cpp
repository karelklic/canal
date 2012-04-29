#include "CommandBreak.h"
#include "Commands.h"
#include "State.h"
#include <cstdio>

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
        puts("No program specified.  Use the \"file\" command.");
        return;
    }

    if (args.size() == 1)
    {
        // TODO: Add breakpoint to current function.
        return;
    }

    for (std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); ++it)
        mCommands.getState()->addFunctionBreakpoint(*it);
}
