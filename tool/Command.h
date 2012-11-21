#ifndef CANAL_COMMAND_H
#define CANAL_COMMAND_H

#include "Prereq.h"
#include <string>
#include <vector>

class Commands;

class Command
{
protected:
    std::string mName;
    std::string mShortcut;
    std::string mHelpLine;
    std::string mHelp;
    Commands &mCommands;

public:
    Command(const std::string &name,
            const std::string &shortcut,
            const std::string &helpLine,
            const std::string &help,
            Commands &commands);

    /// Standard virtual destructor.
    virtual ~Command() {}

    // User printable name of the function.
    const std::string &getName() const { return mName; }
    // Command shortcut.  Empty string if command has none.
    const std::string &getShortcut() const { return mShortcut; }
    // Short explanation of the command.
    const std::string &getHelpLine() const { return mHelpLine; }
    // Long explanation of the command.
    const std::string &getHelp() const { return mHelp; }

    // Finds completion options for given arguments.
    virtual std::vector<std::string> getCompletionMatches(
        const std::vector<std::string> &args,
        int pointArg,
        int pointArgOffset) const;

    /// @brief Perform the command.
    /// @param args
    ///   First argument is the name of the command.
    virtual void run(const std::vector<std::string> &args) = 0;
};

#endif
