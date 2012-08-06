#include "CommandPrint.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include "../lib/SlotTracker.h"
#include "../lib/Stack.h"
#include "../lib/Value.h"
#include "../lib/Utils.h"
#include <llvm/ValueSymbolTable.h>
#include <llvm/Module.h>
#include <cstdio>
#include <sstream>

CommandPrint::CommandPrint(Commands &commands)
    : Command("print",
              "p",
              "Print value of a variable",
              "Print value of a variable.",
              commands)
{
}

static void
filterPlaceValueMap(const Canal::PlaceValueMap &map,
                    Canal::SlotTracker &slotTracker,
                    const char *prefix,
                    const std::string &arg,
                    bool addFunctionName,
                    std::vector<std::string> &result)
{
    Canal::PlaceValueMap::const_iterator it = map.begin();
    for (; it != map.end(); ++it)
    {
        std::stringstream name;
        name << prefix;
        if (addFunctionName)
        {
            const llvm::Instruction &instruction =
                llvm::cast<llvm::Instruction>(*it->first);
            const llvm::Function &f = *instruction.getParent()->getParent();
            name << f.getName().str() << ":";
        }

        name << Canal::getName(*it->first, slotTracker);
        if (0 == strncmp(name.str().c_str(), arg.c_str(), arg.size()))
            result.push_back(name.str());
    }
}

std::vector<std::string>
CommandPrint::getCompletionMatches(const std::vector<std::string> &args,
                                   int pointArg,
                                   int pointArgOffset) const
{
    std::vector<std::string> result;
    State *state = mCommands.getState();
    if (!state || !state->isInterpreting())
        return result;

    std::string arg(args[pointArg].substr(0, pointArgOffset));

    Canal::State &curState = state->getStack().getCurrentState();

    filterPlaceValueMap(curState.getGlobalVariables(),
                        state->getSlotTracker(),
                        "@", arg, false, result);
    filterPlaceValueMap(curState.getGlobalBlocks(),
                        state->getSlotTracker(),
                        "@^", arg, true, result);
    filterPlaceValueMap(curState.getFunctionVariables(),
                        state->getSlotTracker(),
                        "%", arg, false, result);
    filterPlaceValueMap(curState.getFunctionBlocks(),
                        state->getSlotTracker(),
                        "%^", arg, false, result);

    return result;
}

static void
printVariable(const std::string &fullName, State &state)
{
    if (fullName.size() == 0)
        return;

    if ((fullName[0] != '%' && fullName[0] != '@') ||
        fullName.size() == 1 ||
        (fullName.size() == 2 && fullName[1] == '^'))
    {
        printf("Parameter \"%s\": invalid format.\n", fullName.c_str());
        return;
    }

    bool isBlock = (fullName.size() > 1 && fullName[1] == '^');

    const llvm::Value *position;
    bool isNumber;
    std::string name(fullName.substr(isBlock ? 2 : 1));

    std::string functionName;
    if (isBlock)
    {
        size_t sep = name.find(":");
        if (std::string::npos != sep)
        {
            functionName = name.substr(0, sep);
            name = name.substr(sep + 1);
        }
    }

    unsigned pos = stringToUnsigned(name.c_str(), isNumber);

    Canal::Stack &stack = state.getStack();
    Canal::State &currentState = stack.getCurrentState();
    Canal::SlotTracker &slotTracker = state.getSlotTracker();

    const llvm::Function *function = &stack.getCurrentFunction();
    if (!functionName.empty())
    {
        function = state.getModule().getFunction(functionName);
        CANAL_ASSERT_MSG(function, "Cannot found already used function!");
    }
    slotTracker.setActiveFunction(*function);

    if (isNumber)
    {
        if (fullName[0] == '%' || isBlock)
            position = slotTracker.getLocalSlot(pos);
        else
            position = slotTracker.getGlobalSlot(pos);
    }
    else
    {
        if (fullName[0] == '%' || isBlock)
            position = function->getValueSymbolTable().lookup(name);
        else
            position = state.getModule().getValueSymbolTable().lookup(name);
    }

    if (!position)
    {
        printf("Parameter \"%s\": does not exist in current scope.\n",
               fullName.c_str());
        return;
    }

    Canal::Value *value = NULL;
    if (isBlock)
        value = currentState.findBlock(*position);
    else
        value = currentState.findVariable(*position);

    if (!value)
    {
        printf("%s = uninitialized\n", fullName.c_str());
        return;
    }

    printf("%s = %s\n", fullName.c_str(),
           Canal::indentExceptFirstLine(value->toString(),
                                        fullName.size() + 3).c_str());
}

void
CommandPrint::run(const std::vector<std::string> &args)
{
    State *state = mCommands.getState();
    if (!state || !state->isInterpreting())
    {
        puts("No module is being interpreted.");
        return;
    }

    if (args.size() <= 1)
    {
        puts("Provide variable or constant name as \"print\" argument.");
        return;
    }

    if (args[1] == "%" || args[1] == "@" || args[1] == "@^" || args[1] == "%^")
    {
        std::vector<std::string> variables;
        Canal::State &curState = state->getStack().getCurrentState();

        bool addFunctionName = (args[1] == "@^");
        const Canal::PlaceValueMap *map = NULL;
        if (args[1] == "%")
            map = &curState.getFunctionVariables();
        else if (args[1] == "@")
            map = &curState.getGlobalVariables();
        else if (args[1] == "@^")
            map = &curState.getGlobalBlocks();
        else if (args[1] == "%^")
            map = &curState.getFunctionBlocks();
        else
            CANAL_DIE();

        filterPlaceValueMap(*map, state->getSlotTracker(),
                            args[1].c_str(), args[1],
                            addFunctionName, variables);

        std::vector<std::string>::const_iterator it = variables.begin();
        for (; it != variables.end(); ++it)
            printVariable(*it, *state);
    }
    else
    {
        std::vector<std::string>::const_iterator it = args.begin() + 1;
        for (; it != args.end(); ++it)
            printVariable(*it, *state);
    }
}
