#include "CommandSet.h"
#include "lib/WideningDataIterationCount.h"

CommandSet::CommandSet(Commands &commands)
    : Command("set",
              "",
              "Set one of the canal options",
              "Set one of the canal options",
              commands)
{
    mOptions["widening-iterations"] = WideningIterations;
}

std::vector<std::string>
CommandSet::getCompletionMatches(const std::vector<std::string> &args,
                                 int pointArg,
                                 int pointArgOffset) const
{
    std::vector<std::string> result;
    // TODO: implement set command completion
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
    
    Canal::Widening::count = atoi(args[2].c_str());
    llvm::outs() << "Widening count set to " << args[2] << ".\n";
}

void
CommandSet::run(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        llvm::outs() << "Arguments required (option name)\n";
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
        default:
            llvm::outs() << "No action defined for the command.\n";
            break;
    }
}
