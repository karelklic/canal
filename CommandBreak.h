#ifndef CANAL_COMMAND_BREAK_H
#define CANAL_COMMAND_BREAK_H

#include "Command.h"

class CommandBreak : public Command
{
public:
    CommandBreak(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_BREAK_H
