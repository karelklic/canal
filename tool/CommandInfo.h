#ifndef CANAL_COMMAND_INFO_H
#define CANAL_COMMAND_INFO_H

#include "Command.h"

class CommandInfo : public Command
{
public:
    CommandInfo(Commands &commands);

    // Implementation of Command::getCompletionMatches().
    virtual std::vector<std::string> getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const;
    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);

protected:
    void infoModule();
};

#endif
