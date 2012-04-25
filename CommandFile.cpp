#include "CommandFile.h"
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRReader.h>
#include "lib/Interpreter.h"
#include <cstdio>
#include <sys/types.h>
#include <dirent.h>

CommandFile::CommandFile(Commands &commands)
    : Command("file",
              "Use FILE as program to be interpreted",
              "",
              commands),
      mInterpreter(NULL)
{
}

std::vector<std::string>
CommandFile::getCompletionMatches(const std::vector<std::string> &args, int pointArg, int pointArgOffset) const
{
    std::vector<std::string> result;
    if (pointArgOffset == 0)
    {
	DIR *dir = opendir(".");
	struct dirent *dirent;
	for (dirent = readdir(dir); dirent != NULL; dirent = readdir(dir))
        {
            if (!dirent || !dirent->d_name)
                continue;
            result.push_back(dirent->d_name);
        }
        closedir(dir);
    }
    else
    {
        std::string arg = args[pointArg].substr(0, pointArgOffset);
	DIR *dir = opendir(".");
	struct dirent *dirent;
	for (dirent = readdir(dir); dirent != NULL; dirent = readdir(dir))
        {
            if (!dirent || !dirent->d_name)
                continue;

            if (0 == strncmp(dirent->d_name, arg.c_str(), arg.length()))
                result.push_back(dirent->d_name);
        }
    }

    return result;
}

void
CommandFile::run(const std::vector<std::string> &args)
{
    if (args.size() > 2)
    {
        puts("Too many parameters.");
        return;
    }
    else if (args.size() < 2)
    {
        puts("Argument required (module file).");
        return;
    }

    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::SMDiagnostic err;
    llvm::Module *module = llvm::ParseIRFile(args[1], err, context);
    if (!module)
    {
        puts("Failed to load the module.");
        return;
    }

    if (mInterpreter)
    {
        // TODO: Query user.
        delete mInterpreter;
    }

    mInterpreter = new Canal::Interpreter(*module);
    puts("Module loaded.");
}
