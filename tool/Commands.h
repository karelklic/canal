#ifndef CANAL_COMMANDS_H
#define CANAL_COMMANDS_H

#include "Prereq.h"
#include <string>
#include <vector>
#include <map>

class Command;
class CommandFile;
class State;

class Commands
{
    std::string mLastCommand;
    State *mState;

public:
    std::vector<Command*> mCommandList;
    typedef std::map<std::string, Command*> CommandMap;
    CommandMap mCommandMap;

public:
    Commands();
    ~Commands();

    // Finds completion options for given text that represents an
    // incomplete command line.
    std::vector<std::string> getCompletionMatches(
        const std::string &text, int point) const;

    std::vector<std::string> getCommandMatches(
        const std::string &command) const;

    // Runs a command line that should contain a command name and
    // command arguments.  Does nothing on empty line.
    void executeLine(const std::string &line);

    // Finds command by its name.  Returns NULL if no such command is
    // found.
    Command *getCommand(const std::string &name);
    const Command *getCommand(const std::string &name) const;

    State *getState()
    {
        return mState;
    }

    const State *getState() const
    {
        return mState;
    }

    // The new state takes ownership of the module.
    void createState(llvm::Module *module);
};

#endif
