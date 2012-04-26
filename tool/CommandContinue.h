#ifndef CANAL_COMMAND_CONTINUE_H
#define CANAL_COMMAND_CONTINUE_H

#include "Command.h"

class CommandContinue : public Command
{
public:
    CommandContinue(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_CONTINUE_H
