#ifndef CANAL_COMMAND_QUIT_H
#define CANAL_COMMAND_QUIT_H

#include "Command.h"

class CommandQuit : public Command
{
public:
    CommandQuit(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_QUIT_H
