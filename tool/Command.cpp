#include "Command.h"

Command::Command(const std::string &name,
                 const std::string &shortcut,
                 const std::string &helpLine,
                 const std::string &help,
                 Commands &commands)
    : mName(name), mShortcut(shortcut),
      mHelpLine(helpLine), mHelp(help), mCommands(commands)
{
}

std::vector<std::string>
Command::getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const
{
    // No completion by default.
    return std::vector<std::string>();
}
