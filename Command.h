#ifndef CANAL_COMMAND_H
#define CANAL_COMMAND_H

#include <string>
#include <vector>

class Commands;

class Command
{
public:
    Command(const std::string &name,
            const std::string &helpLine,
            const std::string &help,
            Commands &commands);

    // User printable name of the function.
    const std::string &getName() const { return mName; }
    // Short explanation of the command.
    const std::string &getHelpLine() const { return mHelpLine; }
    // Long explanation of the command.
    const std::string &getHelp() const { return mHelp; }

    // Finds completion options for given arguments.
    virtual std::vector<std::string> getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const;

    // Function to call to do the job
    virtual void run(const std::vector<std::string> &args) = 0;
protected:
    std::string mName;
    std::string mHelpLine;
    std::string mHelp;
    Commands &mCommands;
};

#endif
