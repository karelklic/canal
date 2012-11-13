#ifndef CANAL_COMMAND_PRINT_H
#define CANAL_COMMAND_PRINT_H

#include "Command.h"


class CommandPrint : public Command
{
public:
    /// Standard constructor.
    CommandPrint(Commands &commands);

    /// @note
    ///   Implements Command::getCompletionMatches().
    virtual std::vector<std::string> getCompletionMatches(
        const std::vector<std::string> &args,
        int pointArg,
        int pointArgOffset) const;

    /// @brief
    ///   Prints current values of variables that were requested by
    ///   arguments.
    ///
    /// "@" means all global values are printed. "%" means all local
    /// variables of current function are printed.
    ///
    /// @param args
    ///   First argument is the name of the command.
    ///
    /// @note
    ///   Implements Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_PRINT_H
