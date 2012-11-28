#include "CommandSet.h"
#include "lib/IntegerEnumeration.h"
#include "lib/InterpreterOperationsCallback.h"
#include "lib/WideningDataIterationCount.h"

CommandSet::CommandSet(Commands &commands)
    : Command("set",
              "",
              "Set one of the canal options",
              "Set one of the canal options",
              commands)
{
    mOptions["widening-iterations"] = CommandSet::WideningIterations;
    mOptions["no-missing"] = CommandSet::NoMissing;
    mOptions["enumeration-threshold"] = CommandSet::EnumerationThreshold;
}

std::vector<std::string>
CommandSet::getCompletionMatches(const std::vector<std::string> &args,
                                 int pointArg,
                                 int pointArgOffset) const
{
    std::vector<std::string> result;
    std::string arg = args[pointArg].substr(0, pointArgOffset);
    OptionMap::const_iterator it = mOptions.begin(),
        itend = mOptions.end();

    std::string optionName;
    for (; it != itend; ++it)
    {
        optionName = it->first;
        if (optionName.substr(0, arg.length()) == arg)
            result.push_back(optionName);
    }

    std::sort(result.begin(), result.end());
    return result;
}

static bool
isNumber(const std::string &s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return it == s.end();
}

static void
setWideningIterations(const std::vector<std::string> &args)
{
    if (args.size() < 3)
    {
        llvm::outs() << "Number of iterations must be specified.\n";
        return;
    }

    if (!isNumber(args[2]))
    {
        llvm::outs() << "Iteration count must be a number.\n";
        return;
    }

    Canal::Widening::DataIterationCount::ITERATION_COUNT = std::atoi(args[2].c_str());
    llvm::outs() << "Widening count set to " << args[2] << ".\n";
}

static void
setNoMissing()
{
    Canal::Interpreter::printMissing = false;
    llvm::outs() << "Not printing missing functions.\n";
}

static void
setEnumerationThreshold(const std::vector<std::string> &args)
{
    if (args.size() < 3)
    {
        llvm::outs() << "Enumeration threshold must be specified.\n";
        return;
    }

    if (!isNumber(args[2]))
    {
        llvm::outs() << "Enumeration threshold must be a number.\n";
        return;
    }

    Canal::Integer::Enumeration::ENUMERATION_THRESHOLD = std::atoi(args[2].c_str());
    llvm::outs() << "Enumeration threshold set to " << args[2] << ".\n";
}

void
CommandSet::run(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        llvm::outs() << "Arguments required (option name).\n";
        return;
    }

    OptionMap::iterator it = mOptions.find(args[1]);
    if (it == mOptions.end())
    {
        llvm::outs() << "Unknown option.\n";
        return;
    }

    CommandSet::Option option = it->second;

    switch (option)
    {
        case WideningIterations:
            setWideningIterations(args);
            break;
        case NoMissing:
            setNoMissing();
            break;
        case EnumerationThreshold:
            setEnumerationThreshold(args);
            break;
        default:
            llvm::outs() << "No action defined for the command.\n";
            break;
    }
}
