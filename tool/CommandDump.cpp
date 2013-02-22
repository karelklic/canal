#include "CommandDump.h"
#include "State.h"
#include "Commands.h"
#include "Utils.h"
#include <cstdio>

CommandDump::CommandDump(Commands &commands)
    : Command("dump",
              "",
              "Dump the state of the program interpretation to a file",
              "Dump the state of the program interpretation to a file",
              commands)
{
}

std::vector<std::string>
CommandDump::getCompletionMatches(const std::vector<std::string> &args,
                                  int pointArg,
                                  int pointArgOffset) const
{
    std::vector<std::string> result;
    if (pointArg > 1)
        return result;

    std::string arg = args[pointArg].substr(0, pointArgOffset);

    // TODO: file completion here

    return result;
}

void
CommandDump::run(const std::vector<std::string> &args)
{
    if (args.size() > 2)
    {
        llvm::outs() << "Too many parameters.\n";
        return;
    }

    if (args.size() < 2)
    {
        mCommands.executeLine("help dump");
        return;
    }

    if (!mCommands.getState())
    {
        llvm::outs() << "No program is loaded.  Load a program first.\n";
        return;
    }

    FILE *fp = fopen(args[1].c_str(), "r");
    if (fp)
    {
        fclose(fp);
        llvm::outs() << "The file already exists.\n";
        bool agreed = askYesNo("Replace it?");
        if (!agreed)
        {
            llvm::outs() << "Not saved.\n";
            return;
        }
    }

    std::string error;
    llvm::raw_fd_ostream out(args[1].c_str(), error);
    if (out.has_error())
    {
        llvm::outs() << "Failed to open the file:\n";
        llvm::outs() << error << "\n";
        return;
    }

    //Dump as .s file -> save as metadata
    if (args[1].length() > 2 && args[1].substr(args[1].length() - 2) == ".s") {
        mCommands.getState()->getInterpreter().dumpToMetadata();
        mCommands.getState()->getModule().print(out, NULL);
    }
    else {
        out << mCommands.getState()->getInterpreter().toString();
    }
    out.flush();

    llvm::outs() << "Interpretation state saved.\n";
}
