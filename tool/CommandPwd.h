#ifndef CANAL_COMMAND_PWD_H
#define CANAL_COMMAND_PWD_H

#include "Command.h"

class CommandPwd : public Command
{
public:
    CommandPwd(Commands &commands);

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_PWD_H
