#ifndef CANAL_COMMAND_PRINT_H
#define CANAL_COMMAND_PRINT_H

#include "Command.h"

class CommandPrint : public Command
{
public:
    CommandPrint(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_PRINT_H
