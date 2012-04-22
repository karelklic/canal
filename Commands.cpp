#include "Commands.h"
#include "lib/Utils.h"
#include <cstring>
#include <cstdio>

CdCommand::CdCommand()
{
    mName = "cd";
    mDocLine = "Set working directory to DIR for interpreter";
}

void
CdCommand::run(const std::vector<std::string> args)
{
    CANAL_NOT_IMPLEMENTED();
}

HelpCommand::HelpCommand()
{
    mName = "help";
    mDocLine = "Print list of commands";
}

void
HelpCommand::run(const std::vector<std::string> args)
{
    puts("List of commands:");
    puts("");
    puts("Specifying files");
    puts("cd -- Set working directory to DIR for interpreter");
    puts("file -- Use FILE as program to be interpreted");
    puts("pwd -- Print working directory");
    puts("");
    puts("Running the program");
    puts("continue -- Continue the program being interpreted");
    puts("finish -- Execute until selected stack frame returns");
    puts("nexti -- Step one instruction on current level");
    puts("run -- Start program interpretation");
    puts("stepi -- Step one instruction exactly");
    puts("start -- Interpret the program until the beginning of the main procedure");
    puts("");
    puts("Examining the stack");
    puts("backtrace -- Print backtrace of all stack frames");
    puts("");
    puts("Support facilities");
    puts("help -- Print list of commands");
    puts("quit -- exit Canal");
}

CommandList::CommandList()
{
    mCommands.push_back(new CdCommand());
    mCommands.push_back(new HelpCommand());
}

CommandList::~CommandList()
{
    for (std::vector<Command*>::const_iterator it = mCommands.begin(); it != mCommands.end(); ++it)
    {
        delete *it;
    }
}

std::vector<const char*>
CommandList::getCompletionMatches(const char *text) const
{
    std::vector<const char*> matches;
    for (std::vector<Command*>::const_iterator it = mCommands.begin(); it != mCommands.end(); ++it)
    {
        if (0 == strncmp((*it)->getName().c_str(), text, strlen(text)))
            matches.push_back((*it)->getName().c_str());
    }

    return matches;
}

void
CommandList::executeLine(char *line)
{
    std::vector<std::string> args;

    char *pos = strtok(line, " ");
    while (pos != NULL)
    {
        args.push_back(pos);
        pos = strtok(NULL, " ");
    }

    if (args.empty())
        return;

    for (std::vector<Command*>::const_iterator it = mCommands.begin(); it != mCommands.end(); ++it)
    {
        if ((*it)->getName() == args[0])
        {
            (*it)->run(args);
            return;
        }
    }

    printf("Undefined command: \"%s\".  Try \"help\".\n", args[0].c_str());
}
