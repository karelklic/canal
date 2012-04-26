#ifndef CANAL_COMMAND_NEXT_H
#define CANAL_COMMAND_NEXT_H

#include "Command.h"

class CommandNext : public Command
{
public:
    CommandNext(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_NEXT_H
