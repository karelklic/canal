#ifndef CANAL_COMMAND_HELP_H
#define CANAL_COMMAND_HELP_H

#include "Command.h"

class CommandHelp : public Command
{
public:
    CommandHelp(Commands &commands);

    // Implementation of Command::getCompletionMatches().
    virtual std::vector<std::string> getCompletionMatches(
        const std::vector<std::string> &args,
        int pointArg,
        int pointArgOffset) const;

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);

private:
    void allCommandsHelp();
};

#endif // CANAL_COMMAND_HELP_H
