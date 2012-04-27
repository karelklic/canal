#ifndef CANAL_COMMAND_SHELL_H
#define CANAL_COMMAND_SHELL_H

#include "Command.h"

class CommandShell : public Command
{
public:
    CommandShell(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_SHELL_H
