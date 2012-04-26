#ifndef CANAL_COMMAND_STEP_H
#define CANAL_COMMAND_STEP_H

#include "Command.h"

class CommandStep : public Command
{
public:
    CommandStep(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_STEP_H
