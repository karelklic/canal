#ifndef CANAL_COMMAND_START_H
#define CANAL_COMMAND_START_H

#include "Command.h"

class CommandStart : public Command
{
public:
    CommandStart(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_START_H
