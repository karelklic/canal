#include "Commands.h"
#include "CommandBacktrace.h"
#include "CommandBreak.h"
#include "CommandCd.h"
#include "CommandContinue.h"
#include "CommandFile.h"
#include "CommandFinish.h"
#include "CommandHelp.h"
#include "CommandInfo.h"
#include "CommandNext.h"
#include "CommandPrint.h"
#include "CommandPwd.h"
#include "CommandQuit.h"
#include "CommandRun.h"
#include "CommandShow.h"
#include "CommandStart.h"
#include "CommandStep.h"
#include <cstring>
#include <cstdio>

Commands::Commands()
{
    mCommandList.push_back(new CommandBacktrace(*this));
    mCommandList.push_back(new CommandBreak(*this));
    mCommandList.push_back(new CommandCd(*this));
    mCommandList.push_back(new CommandContinue(*this));
    mCommandList.push_back(new CommandFile(*this));
    mCommandList.push_back(new CommandFinish(*this));
    mCommandList.push_back(new CommandHelp(*this));
    mCommandList.push_back(new CommandInfo(*this));
    mCommandList.push_back(new CommandNext(*this));
    mCommandList.push_back(new CommandPrint(*this));
    mCommandList.push_back(new CommandPwd(*this));
    mCommandList.push_back(new CommandQuit(*this));
    mCommandList.push_back(new CommandRun(*this));
    mCommandList.push_back(new CommandShow(*this));
    mCommandList.push_back(new CommandStart(*this));
    mCommandList.push_back(new CommandStep(*this));

    for (std::vector<Command*>::const_iterator it = mCommandList.begin(); it != mCommandList.end(); ++it)
    {
        mCommandMap.insert(CommandMap::value_type((*it)->getName(), *it));
    }
}

Commands::~Commands()
{
    for (std::vector<Command*>::const_iterator it = mCommandList.begin(); it != mCommandList.end(); ++it)
    {
        delete *it;
    }
}

std::vector<std::string>
Commands::getCompletionMatches(const std::string &text, int point) const
{
    // Split the text into tokens and determine where the point is.
    std::vector<std::string> args;
    int pointArg = -1;
    int pointArgOffset = 0;
    char textString[text.length() + 1];
    strcpy(textString, text.c_str());
    char *pos = strtok(textString, " \n");
    while (pos != NULL)
    {
        if (point >= (pos - textString) &&
            point <= (pos - textString + strlen(pos)))
        {
            pointArg = args.size();
            pointArgOffset = point - (pos - textString);
        }
        else if (pointArg == -1 &&
                 point <= (pos - textString + strlen(pos)))
        {
            pointArg = args.size();
        }

        args.push_back(pos);
        pos = strtok(NULL, " \n");
    }

    if (pointArg == -1)
    {
        pointArg = args.size();
        args.push_back("");
    }

    // printf("\"%s\" %d ->  %d %d\n", text.c_str(), point, pointArg, pointArgOffset);

    // If the point is in the command, complete the command.
    if (pointArg == 0)
    {
        // We do not know the command.  Let's complete its name.
        return getCommandMatches(args[0].substr(0, pointArgOffset));
    }
    else // Complete command arguments.
    {
        const Command *command = getCommand(args[0]);
        if (command)
        {
            // We know the command.  Ask the command for completion of its
            // argumens.
            return command->getCompletionMatches(args, pointArg, pointArgOffset);
        }
    }

    return std::vector<std::string>();;
}

std::vector<std::string>
Commands::getCommandMatches(const std::string &command) const
{
    std::vector<std::string> result;
    for (std::vector<Command*>::const_iterator it = mCommandList.begin(); it != mCommandList.end(); ++it)
    {
        if (0 == strncmp((*it)->getName().c_str(), command.c_str(), command.length()))
            result.push_back((*it)->getName());
    }
    return result;
}

void
Commands::executeLine(const std::string &line)
{
    std::vector<std::string> args;

    // Split the line into tokens (separated by space)
    char lineString[line.length() + 1];
    strcpy(lineString, line.c_str());
    char *pos = strtok(lineString, " ");
    while (pos != NULL)
    {
        args.push_back(pos);
        pos = strtok(NULL, " ");
    }

    // Handle empty line.
    if (args.empty())
        return;

    // Find the command to be run.  Run the command.
    for (std::vector<Command*>::const_iterator it = mCommandList.begin(); it != mCommandList.end(); ++it)
    {
        if ((*it)->getName() == args[0])
        {
            (*it)->run(args);
            return;
        }
    }

    // Failed to find a command.
    printf("Undefined command: \"%s\".  Try \"help\".\n", args[0].c_str());
}

Command *
Commands::getCommand(const std::string &name)
{
    CommandMap::iterator it = mCommandMap.find(name);
    return it == mCommandMap.end() ? NULL : it->second;
}

const Command *
Commands::getCommand(const std::string &name) const
{
    CommandMap::const_iterator it = mCommandMap.find(name);
    return it == mCommandMap.end() ? NULL : it->second;
}
