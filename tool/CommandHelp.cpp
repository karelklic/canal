#include "CommandHelp.h"
#include "Commands.h"
#include "lib/Utils.h"

CommandHelp::CommandHelp(Commands &commands)
    : Command("help",
              "h",
              "Print list of commands",
              "Print list of commands.",
              commands)
{
}

std::vector<std::string>
CommandHelp::getCompletionMatches(const std::vector<std::string> &args,
                                  int pointArg,
                                  int pointArgOffset) const
{
    return mCommands.getCommandMatches(
        args[pointArg].substr(0, pointArgOffset));
}

void
CommandHelp::run(const std::vector<std::string> &args)
{
    if (args.size() == 1)
    {
        allCommandsHelp();
        return;
    }

    // Command-specific help.
    CANAL_ASSERT(args.size() > 1);
    Command *command = mCommands.getCommand(args[1]);
    if (!command)
    {
        std::vector<std::string> matches =
            mCommands.getCompletionMatches(args[1], args[1].length());

        if (matches.size() > 1)
        {
            llvm::outs() << "Ambiguous command \"" << args[1] << "\": ";
            std::vector<std::string>::const_iterator it = matches.begin();
            for (; it != matches.end(); ++it)
            {
                if (it != matches.begin())
                    llvm::outs() << ", ";

                llvm::outs() << *it;
            }

            llvm::outs() << "\n";
            return;
        }
        else if (matches.size() == 1)
        {
            command = mCommands.getCommand(matches[0]);
            CANAL_ASSERT(command);
        }
        else
        {
            // Failed to find a command.
            llvm::outs() << "Undefined command: \"" << args[1] << "\".  "
                         << "Try \"help\".\n";

            return;
        }
    }

    llvm::outs() << command->getHelp() << "\n";
}

static std::string
commandGetStringAndRemove(const std::string &name, std::vector<Command*> &commands)
{
    Canal::StringStream ss;
    std::vector<Command*>::iterator it = commands.begin();
    for (; it != commands.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            ss << name << " -- " << (*it)->getHelpLine() << "\n";
            commands.erase(it);
            return ss.str();
        }
    }

    CANAL_DIE();
}

void
CommandHelp::allCommandsHelp()
{
    std::vector<Command*> commands = mCommands.mCommandList;
    llvm::outs() << "List of commands:\n"
                 << "\n"
                 << "Making program stop at certain points\n"
                 << commandGetStringAndRemove("break", commands)
                 << "\n"
                 << "Examining data\n"
                 << commandGetStringAndRemove("print", commands)
                 << commandGetStringAndRemove("dump", commands)
                 << commandGetStringAndRemove("set", commands)
                 << "\n"
                 << "Specifying files\n"
                 << commandGetStringAndRemove("cd", commands)
                 << commandGetStringAndRemove("file", commands)
                 << commandGetStringAndRemove("pwd", commands)
                 << "\n"
                 << "Running the program\n"
                 << commandGetStringAndRemove("continue", commands)
                 << commandGetStringAndRemove("finish", commands)
                 << commandGetStringAndRemove("run", commands)
                 << commandGetStringAndRemove("step", commands)
                 << commandGetStringAndRemove("start", commands)
                 << "\n"
                 << "Status inquiries\n"
                 << commandGetStringAndRemove("info", commands)
                 << commandGetStringAndRemove("show", commands)
                 << "\n"
                 << "Support facilities\n"
                 << commandGetStringAndRemove("help", commands)
                 << commandGetStringAndRemove("quit", commands)
                 << "\n"
                 << "Type \"help\" followed by command name for full documentation.\n"
                 << "Command name abbreviations are allowed if unambiguous.\n";

    CANAL_ASSERT(commands.empty());
}
