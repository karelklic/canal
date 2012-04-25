#ifndef CANAL_COMMANDS_H
#define CANAL_COMMANDS_H

#include <string>
#include <vector>
#include <map>

class Command;
class CommandFile;

class Commands
{
public:
    Commands();
    ~Commands();

    // Finds completion options for given text that represents an
    // incomplete command line.
    std::vector<std::string> getCompletionMatches(const std::string &text, int point) const;
    std::vector<std::string> getCommandMatches(const std::string &command) const;

    // Runs a command line that should contain a command name and
    // command arguments.  Does nothing on empty line.
    void executeLine(const std::string &line);

    // Finds command by its name.  Returns NULL if no such command is
    // found.
    Command *getCommand(const std::string &name);
    const Command *getCommand(const std::string &name) const;

    CommandFile &getFile() { return *mFile; }
    const CommandFile &getFile() const { return *mFile; }

public:
    std::vector<Command*> mCommandList;
    typedef std::map<std::string, Command*> CommandMap;
    CommandMap mCommandMap;

protected:
    CommandFile *mFile;
};

#endif
