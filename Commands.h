#ifndef CANAL_COMMANDS_H
#define CANAL_COMMANDS_H

#include <vector>
#include <string>

class Command
{
public:
    // User printable name of the function
    const std::string &getName() const { return mName; }

    // Documentation line for this function
    const std::string &getDocLine() const { return mDocLine; }

    // Function to call to do the job
    virtual void run(const std::vector<std::string> args) = 0;

protected:
    std::string mName;
    std::string mDocLine;
};

class CdCommand : public Command
{
public:
    CdCommand();

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> args);
};

class HelpCommand : public Command
{
public:
    HelpCommand();

    // Implementation of Command::run().
    virtual void run(const std::vector<std::string> args);
};

class CommandList
{
public:
    CommandList();
    ~CommandList();

    // Do not delete the strings returned in the list.
    std::vector<const char*> getCompletionMatches(const char *text) const;

    void executeLine(char *line);

public:
    std::vector<Command*> mCommands;
};

#endif // CANAL_COMMANDS_H
