#include "CommandPrint.h"
#include "Commands.h"
#include "State.h"
#include "Utils.h"
#include "lib/SlotTracker.h"
#include "lib/Domain.h"
#include "lib/Utils.h"
#include "lib/InterpreterFunction.h"
#include "lib/StateMap.h"

CommandPrint::CommandPrint(Commands &commands)
    : Command("print",
              "p",
              "Print value of a variable",
              "Print value of a variable.",
              commands)
{
}

static void
filterStateMap(const Canal::StateMap &map,
               Canal::SlotTracker &slotTracker,
               const char *prefix,
               const std::string &arg,
               bool addFunctionName,
               std::vector<std::string> &result)
{
    Canal::StateMap::const_iterator it = map.begin();
    for (; it != map.end(); ++it)
    {
        Canal::StringStream name;
        name << prefix;
        if (addFunctionName)
        {
            const llvm::Instruction *instruction =
                Canal::dynCast<llvm::Instruction>(it->first);

            if (instruction)
            {
                const llvm::Function &function =
                    *instruction->getParent()->getParent();

                name << function.getName().str() << ":";
            }
            else
                CANAL_ASSERT_MSG(llvm::isa<llvm::GlobalVariable>(it->first),
                                 "Unexpected entity in a place-variable map.");
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
    if (!state)
        return result;

    std::string arg(args[pointArg].substr(0, pointArgOffset));
    const Canal::State &curState = state->getInterpreter().getCurrentState();

    if (arg.empty() || arg == "%")
    {
        result.push_back("%");
        result.push_back("%^");
    }

    if (arg.empty() || arg == "@")
    {
        result.push_back("@");
        result.push_back("@^");
    }

    filterStateMap(curState.getGlobalVariables(),
                   state->getSlotTracker(),
                   "@", arg, false, result);
    filterStateMap(curState.getGlobalBlocks(),
                   state->getSlotTracker(),
                   "@^", arg, true, result);
    filterStateMap(curState.getFunctionVariables(),
                   state->getSlotTracker(),
                   "%", arg, false, result);
    filterStateMap(curState.getFunctionBlocks(),
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
        llvm::outs() << "Parameter \"" << fullName << "\": invalid format.\n";
        return;
    }

    bool isBlock = (fullName.size() > 1 && fullName[1] == '^');

    const llvm::Value *position;
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

    bool isNumber;
    unsigned pos = stringToUnsigned(name.c_str(), isNumber);

    const Canal::Interpreter::Interpreter &interpreter =
        state.getInterpreter();

    const Canal::State &currentState = interpreter.getCurrentState();
    Canal::SlotTracker &slotTracker = interpreter.getSlotTracker();

    const Canal::Interpreter::Function *function =
        &interpreter.getCurrentFunction();

    if (!functionName.empty())
    {
        function = interpreter.getModule().getFunction(functionName);
        if (!function)
        {
            llvm::outs() << "Function \"" << functionName << "\" not found.\n";
            return;
        }
    }
    slotTracker.setActiveFunction(function->getLlvmFunction());

    if (isNumber)
    {
        if (fullName[0] == '%' || (isBlock && !functionName.empty()))
            position = slotTracker.getLocalSlot(pos);
        else
            position = slotTracker.getGlobalSlot(pos);
    }
    else
    {
        if (fullName[0] == '%' || (isBlock && !functionName.empty()))
            position = function->getLlvmFunction().getValueSymbolTable().lookup(name);
        else
            position = state.getModule().getValueSymbolTable().lookup(name);
    }

    if (!position)
    {
        llvm::outs() << "Parameter \"" << fullName
                     << "\": does not exist in current scope.\n";

        return;
    }

    const Canal::Domain *value = NULL;
    if (isBlock)
        value = currentState.findBlock(*position);
    else
        value = currentState.findVariable(*position);

    if (!value)
    {
        llvm::outs() << fullName << " = uninitialized\n";
        return;
    }

    llvm::outs() << fullName << " = "
                 << Canal::indentExceptFirstLine(value->toString(),
                                                 fullName.size() + 3);
}

void
CommandPrint::run(const std::vector<std::string> &args)
{
    State *state = mCommands.getState();
    if (!state)
    {
        llvm::outs() << "No module is being interpreted.\n";
        return;
    }

    if (args.size() <= 1)
    {
        llvm::outs() << "Provide variable or constant name as \"print\" argument.\n";
        return;
    }

    if (args[1] == "%" || args[1] == "@" || args[1] == "@^" || args[1] == "%^")
    {
        std::vector<std::string> variables;
        const Canal::State &curState =
            state->getInterpreter().getCurrentState();

        bool addFunctionName = (args[1] == "@^");
        const Canal::StateMap *map = NULL;
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

        filterStateMap(*map,
                       state->getSlotTracker(),
                       args[1].c_str(),
                       args[1],
                       addFunctionName,
                       variables);

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
