#include "CommandHelp.h"
#include "Commands.h"
#include "lib/Utils.h"
#include <cstdio>

CommandHelp::CommandHelp(Commands &commands)
    : Command("help",
              "Print list of commands",
              "",
              commands)
{
}

std::vector<std::string>
CommandHelp::getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const
{
    return mCommands.getCommandMatches(args[pointArg].substr(0, pointArgOffset));
}

static void
printCommand(const std::string &name, std::vector<Command*> &commands)
{
    for (std::vector<Command*>::iterator it = commands.begin(); it != commands.end(); ++it)
    {
        if ((*it)->getName() == name)
        {
            printf("%s -- %s\n", name.c_str(), (*it)->getHelpLine().c_str());
            commands.erase(it);
            return;
        }
    }

    CANAL_DIE();
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
        std::vector<std::string> matches = mCommands.getCompletionMatches(args[1], args[1].length());
        if (matches.size() > 1)
        {
            printf("Ambiguous command \"%s\": ", args[1].c_str());
            for (std::vector<std::string>::const_iterator it = matches.begin(); it != matches.end(); ++it)
            {
                if (it != matches.begin())
                    printf(", ");
                printf(it->c_str());
            }
            puts("");
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
            printf("Undefined command: \"%s\".  Try \"help\".\n", args[1].c_str());
            return;
        }
    }

    puts(command->getHelp().c_str());
}

void
CommandHelp::allCommandsHelp()
{
    std::vector<Command*> commands = mCommands.mCommandList;
    puts("List of commands:");
    puts("");
    puts("Making program stop at certain points");
    printCommand("break", commands);
    puts("");
    puts("Examining data");
    printCommand("print", commands);
    puts("");
    puts("Specifying files");
    printCommand("cd", commands);
    printCommand("file", commands);
    printCommand("pwd", commands);
    puts("");
    puts("Running the program");
    printCommand("continue", commands);
    printCommand("finish", commands);
    printCommand("next", commands);
    printCommand("run", commands);
    printCommand("step", commands);
    printCommand("start", commands);
    puts("");
    puts("Examining the stack");
    printCommand("backtrace", commands);
    puts("");
    puts("Status inquiries");
    printCommand("info", commands);
    printCommand("show", commands);
    puts("");
    puts("Support facilities");
    printCommand("help", commands);
    printCommand("quit", commands);
    puts("");
    puts("Type \"help\" followed by command name for full documentation.");
    puts("Command name abbreviations are allowed if unambiguous.");
    CANAL_ASSERT(commands.empty());
}
