#ifndef CANAL_COMMAND_SHOW_H
#define CANAL_COMMAND_SHOW_H

#include "Command.h"

class CommandShow : public Command
{
public:
    CommandShow(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_SHOW_H
