#ifndef CANAL_COMMAND_RUN_H
#define CANAL_COMMAND_RUN_H

#include "Command.h"

class CommandRun : public Command
{
public:
    CommandRun(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_RUN_H
