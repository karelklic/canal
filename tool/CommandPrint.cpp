#include "CommandPrint.h"
#include "Commands.h"
#include "State.h"
#include "SlotTracker.h"
#include "Utils.h"
#include "../lib/Stack.h"
#include "../lib/Value.h"
#include "../lib/Utils.h"
#include <cstdio>

CommandPrint::CommandPrint(Commands &commands)
    : Command("print",
              "Print value of a variable",
              "",
              commands)
{
}

void
CommandPrint::run(const std::vector<std::string> &args)
{
    if (!mCommands.mState || !mCommands.mState->isInterpreting())
    {
        puts("No module is being interpreted.");
        return;
    }

    if (args.size() <= 1)
    {
        puts("Provide variable or constant name as \"print\" argument.");
        return;
    }

    Canal::State &state = mCommands.mState->mStack->getCurrentState();
    const llvm::Function &function = mCommands.mState->mStack->getCurrentFunction();
    mCommands.mState->mSlotTracker->setActiveFunction(&function);

    for (std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); ++it)
    {
        if (it->size() == 0)
            continue;
        if ((*it)[0] != '%' && (*it)[0] != '@')
        {
            printf("Parameter \"%s\": invalid format.\n", it->c_str());
            continue;
        }

        bool success;
        unsigned pos = stringToUnsigned(it->substr(1).c_str(), success);
        if (!success)
        {
            printf("Parameter \"%s\": invalid format.\n", it->c_str());
            continue;
        }

        const llvm::Value *position;
        if ((*it)[0] == '%')
            position = mCommands.mState->mSlotTracker->getLocalSlot(pos);
        else
            position = mCommands.mState->mSlotTracker->getGlobalSlot(pos);

        if (!position)
        {
            printf("Parameter \"%s\": does not exist in current scope.", it->c_str());
            continue;
        }

        Canal::Value *value = state.findVariable(*position);
        if (!value)
        {
            printf("%s = uninitialized\n", it->c_str());
            continue;
        }

        printf("%s = %s\n", it->c_str(), Canal::indentExceptFirstLine(value->toString(), it->size() + 3).c_str());
    }
}
