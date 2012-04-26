#ifndef CANAL_COMMAND_BACKTRACE_H
#define CANAL_COMMAND_BACKTRACE_H

#include "Command.h"

class CommandBacktrace : public Command
{
public:
    CommandBacktrace(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_BACKTRACE_H
