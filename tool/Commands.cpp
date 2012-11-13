#include "Commands.h"
#include "CommandBreak.h"
#include "CommandCd.h"
#include "CommandContinue.h"
#include "CommandDump.h"
#include "CommandFile.h"
#include "CommandFinish.h"
#include "CommandHelp.h"
#include "CommandInfo.h"
#include "CommandPrint.h"
#include "CommandPwd.h"
#include "CommandQuit.h"
#include "CommandRun.h"
#include "CommandShell.h"
#include "CommandShow.h"
#include "CommandStart.h"
#include "CommandStep.h"
#include "State.h"
#include "lib/Utils.h"
#include <cstring>
#include <cstdio>

Commands::Commands()
{
    mCommandList.push_back(new CommandBreak(*this));
    mCommandList.push_back(new CommandCd(*this));
    mCommandList.push_back(new CommandContinue(*this));
    mCommandList.push_back(new CommandDump(*this));
    mCommandList.push_back(new CommandFile(*this));
    mCommandList.push_back(new CommandFinish(*this));
    mCommandList.push_back(new CommandHelp(*this));
    mCommandList.push_back(new CommandInfo(*this));
    mCommandList.push_back(new CommandPrint(*this));
    mCommandList.push_back(new CommandPwd(*this));
    mCommandList.push_back(new CommandQuit(*this));
    mCommandList.push_back(new CommandRun(*this));
    mCommandList.push_back(new CommandShell(*this));
    mCommandList.push_back(new CommandShow(*this));
    mCommandList.push_back(new CommandStart(*this));
    mCommandList.push_back(new CommandStep(*this));

    std::vector<Command*>::const_iterator it = mCommandList.begin();
    for (; it != mCommandList.end(); ++it)
    {
        mCommandMap.insert(CommandMap::value_type((*it)->getName(), *it));

        if (!(*it)->getShortcut().empty())
            mCommandMap.insert(CommandMap::value_type((*it)->getShortcut(), *it));
    }
}

Commands::~Commands()
{
    std::vector<Command*>::const_iterator it = mCommandList.begin();
    for (; it != mCommandList.end(); ++it)
        delete *it;
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
    while (pos)
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
            // We know the command.  Ask the command for completion of
            // its argumens.
            return command->getCompletionMatches(args,
                                                 pointArg,
                                                 pointArgOffset);
        }
    }

    return std::vector<std::string>();;
}

std::vector<std::string>
Commands::getCommandMatches(const std::string &command) const
{
    std::vector<std::string> result;
    std::vector<Command*>::const_iterator it = mCommandList.begin();
    for (; it != mCommandList.end(); ++it)
    {
        if (0 == strncmp((*it)->getName().c_str(),
                         command.c_str(),
                         command.length()))
        {
            result.push_back((*it)->getName());
        }
    }

    return result;
}

void
Commands::executeLine(const std::string &line)
{
    std::string commandLine(line.size() == 0 ? mLastCommand : line);
    mLastCommand = commandLine;
    std::vector<std::string> args;

    // Split the line into tokens (separated by space)
    char lineString[commandLine.length() + 1];
    strcpy(lineString, commandLine.c_str());
    char *pos = strtok(lineString, " ");
    while (pos)
    {
        args.push_back(pos);
        pos = strtok(NULL, " ");
    }

    // Handle empty line.
    if (args.empty())
        return;

    // Find matching commands and execute if there's only one
    std::vector<std::string> matchingCommands = getCommandMatches(args[0]);
    if (matchingCommands.size() == 1)
    {
        (getCommand(matchingCommands[0]))->run(args);
        return;
    }
    else
    {
        // Find command with matching shortcut.
        std::vector<Command*>::const_iterator it = mCommandList.begin();
        for (; it != mCommandList.end(); ++it)
        {
            if ((*it)->getShortcut() == args[0])
            {
                (*it)->run(args);
                return;
            }
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

void
Commands::createState(const llvm::Module *module)
{
    CANAL_ASSERT(module && "Module cannot be NULL.");
    delete mState;
    mState = new State(module);
}
