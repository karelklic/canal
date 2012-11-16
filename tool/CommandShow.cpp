#include "CommandShow.h"
#include "State.h"
#include "Commands.h"
#include "lib/InterpreterFunction.h"
#include <cstdio>

CommandShow::CommandShow(Commands &commands)
    : Command("show",
              "",
              "Generic command for showing things about the interpreter",
              "Generic command for showing things about the interpreter.",
              commands)
{
}

std::vector<std::string>
CommandShow::getCompletionMatches(const std::vector<std::string> &args,
                                  int pointArg,
                                  int pointArgOffset) const
{
    std::vector<std::string> result;
    std::string arg = args[pointArg].substr(0, pointArgOffset);

    if (pointArg == 1)
    {
        if (0 == strncmp("iterator", arg.c_str(), arg.size()))
            result.push_back("iterator");

        if (0 == strncmp("module", arg.c_str(), arg.size()))
            result.push_back("module");

        if (0 == strncmp("functions", arg.c_str(), arg.size()))
            result.push_back("functions");

        if (0 == strncmp("function", arg.c_str(), arg.size()))
            result.push_back("function");
    }

    return result;
}

void
CommandShow::run(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        mCommands.executeLine("help show");
        return;
    }

    if (args[1] == "iterator")
        showIterator();
    else if (args[1] == "module")
        showModule();
    else if (args[1] == "functions")
        showFunctions();
    else if (args[1] == "function")
    {
        if (args.size() < 3)
        {
            printf("Missing function name.\n");
            return;
        }
        else if (args.size() > 3)
        {
            printf("Too many function names.\n");
            return;
        }

        showFunction(args[2]);
    }
    else
    {
        printf("Undefined show command: \"%s\".  Try \"help show\".\n",
               args[1].c_str());
    }
}

void
CommandShow::showIterator() const
{
    if (!mCommands.getState())
    {
        puts("No module is loaded.");
        return;
    }

    const Canal::Interpreter::Interpreter &interpreter =
        mCommands.getState()->getInterpreter();
}

void
CommandShow::showModule() const
{
    if (!mCommands.getState())
    {
        puts("No module is loaded.");
        return;
    }

    const Canal::Interpreter::Interpreter &interpreter =
        mCommands.getState()->getInterpreter();

    const Canal::Interpreter::Module &module =
        interpreter.getModule();

    puts("Functions:");
    std::vector<Canal::Interpreter::Function*>::const_iterator it = module.begin();
    for (; it != module.end(); ++it)
    {
        printf("  %s\n", (*it)->getName().str().c_str());
        printf("     Used Memory: %zu\n", (*it)->memoryUsage());
    }
}

void
CommandShow::showFunctions() const
{
}

void
CommandShow::showFunction(const std::string &name) const
{
}
