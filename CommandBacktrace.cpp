#include "CommandBacktrace.h"

CommandBacktrace::CommandBacktrace(Commands &commands)
    : Command("backtrace",
              "Print backtrace of all stack frames",
              "",
              commands)
{
}

void
CommandBacktrace::run(const std::vector<std::string> &args)
{
}
