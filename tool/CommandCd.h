#ifndef CANAL_COMMAND_CD_H
#define CANAL_COMMAND_CD_H

#include "Command.h"

class CommandCd : public Command
{
public:
    CommandCd(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_CD_H
