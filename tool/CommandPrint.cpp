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

std::vector<std::string>
CommandPrint::getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const
{
    std::vector<std::string> result;
    State *state = mCommands.getState();
    if (!state || !state->isInterpreting())
        return result;

    std::string arg(args[pointArg].substr(0, pointArgOffset));
    const llvm::Function &function = state->getStack().getCurrentFunction();
    state->getSlotTracker().setActiveFunction(function);

    // Check function arguments.
    llvm::Function::ArgumentListType::const_iterator it = function.getArgumentList().begin();
    for (; it != function.getArgumentList().end(); ++it)
        considerCompletionMatch(*it, result, arg);

    // Check every instruction in the current function.
    llvm::Function::const_iterator fit = function.begin(), fitend = function.end();
    for (; fit != fitend; ++fit)
    {
        llvm::BasicBlock::const_iterator bit = fit->begin(), bitend = fit->end();
        for (; bit != bitend; ++bit)
            considerCompletionMatch(*bit, result, arg);
    }

    // Check named and unnamed module-level variables.
    const llvm::Module &module = state->getModule();
    for (llvm::Module::const_global_iterator it = module.global_begin(); it != module.global_end(); ++it)
    {
        considerCompletionMatch(*it, result, arg);
    }

    return result;
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

    Canal::Stack &stack = state->getStack();
    Canal::State &currentState = stack.getCurrentState();
    const llvm::Function &function = stack.getCurrentFunction();
    Canal::SlotTracker &slotTracker = state->getSlotTracker();
    slotTracker.setActiveFunction(function);

    for (std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); ++it)
    {
        if (it->size() == 0)
            continue;
        if (((*it)[0] != '%' && (*it)[0] != '@') || it->size() == 1)
        {
            printf("Parameter \"%s\": invalid format.\n", it->c_str());
            continue;
        }

        const llvm::Value *position;
        bool isNumber;
        std::string name(it->substr(1));
        unsigned pos = stringToUnsigned(name.c_str(), isNumber);
        if (isNumber)
        {
            if ((*it)[0] == '%')
                position = slotTracker.getLocalSlot(pos);
            else
                position = slotTracker.getGlobalSlot(pos);
        }
        else
        {
            if ((*it)[0] == '%')
                position = function.getValueSymbolTable().lookup(name);
            else
                position = state->getModule().getValueSymbolTable().lookup(name);
        }

        if (!position)
        {
            printf("Parameter \"%s\": does not exist in current scope.\n", it->c_str());
            continue;
        }

        Canal::Value *value = currentState.findVariable(*position);
        if (!value)
        {
            printf("%s = uninitialized\n", it->c_str());
            continue;
        }

        printf("%s = %s\n", it->c_str(), Canal::indentExceptFirstLine(value->toString(), it->size() + 3).c_str());
    }
}

void
CommandPrint::considerCompletionMatch(const llvm::Value &value, std::vector<std::string> &result, const std::string &prefix) const
{
    std::string name(Canal::getName(value, mCommands.getState()->getSlotTracker()));
    if (name.empty())
        return;

    if (0 == strncmp(name.c_str(), prefix.c_str(), prefix.size()))
        result.push_back(name);
}
