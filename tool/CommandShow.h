#ifndef CANAL_COMMAND_SHOW_H
#define CANAL_COMMAND_SHOW_H

#include "Command.h"

class CommandShow : public Command
{
public:
    CommandShow(Commands &commands);

    // Implementation of Command::getCompletionMatches().
    virtual std::vector<std::string> getCompletionMatches(
        const std::vector<std::string> &args,
        int pointArg,
        int pointArgOffset) const;

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);

private:
    void showIterator() const;
    void showModule() const;
    void showFunctions() const;
    void showFunction(const std::string &name) const;
};

#endif // CANAL_COMMAND_SHOW_H
