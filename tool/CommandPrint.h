#ifndef CANAL_COMMAND_PRINT_H
#define CANAL_COMMAND_PRINT_H

#include "Command.h"

namespace llvm {
class Value;
}

class CommandPrint : public Command
{
public:
    CommandPrint(Commands &commands);

    // Implementation of Command::getCompletionMatches().
    virtual std::vector<std::string> getCompletionMatches(
        const std::vector<std::string> &args,
        int pointArg,
        int pointArgOffset) const;

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> &args);
};

#endif // CANAL_COMMAND_PRINT_H
