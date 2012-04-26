#ifndef CANAL_COMMAND_FINISH_H
#define CANAL_COMMAND_FINISH_H

#include "Command.h"

class CommandFinish : public Command
{
public:
    CommandFinish(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_FINISH_H
