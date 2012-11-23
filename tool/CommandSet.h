#ifndef CANAL_COMMAND_SET_H
#define CANAL_COMMAND_SET_H

#include "Command.h"

class CommandSet : public Command
{
    enum Option
    {
        WideningIterations = 1,
        NoMissing
    };

    typedef std::map<std::string, Option> OptionMap;
    OptionMap mOptions;

public:
    CommandSet(Commands &commands);

    // Implementation of Command::getCompletionMatches().
    virtual std::vector<std::string> getCompletionMatches(
        const std::vector<std::string> &args,
        int pointArg,
        int pointArgOffset) const;


    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_SET_H
