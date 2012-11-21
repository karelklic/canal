#include "CommandCd.h"
#include "lib/Utils.h"
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <wordexp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

CommandCd::CommandCd(Commands &commands)
    : Command("cd",
              "",
              "Set working directory to DIR for interpreter",
              "Set working directory to DIR for interpreter.  "
              "Affects module loading.",
              commands)
{
}

std::vector<std::string>
CommandCd::getCompletionMatches(const std::vector<std::string> &args,
                                int pointArg,
                                int pointArgOffset) const
{
    std::vector<std::string> result;
    std::string arg = args[pointArg].substr(0, pointArgOffset);
    std::string dirPath("./");
    bool defaultDirPath = true;
    size_t dirPos = arg.rfind("/");
    if (dirPos != std::string::npos)
    {
        dirPath = arg.substr(0, dirPos + 1);
        defaultDirPath = false;
        if (arg.length() > dirPos + 1)
            arg = arg.substr(dirPos + 1);
        else
            arg = "";
    }

    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
        return result;

    struct dirent *dirent;
    for (dirent = readdir(dir); dirent != NULL; dirent = readdir(dir))
    {
        if (!dirent || !dirent->d_name)
            continue;

        if (dirent->d_type != DT_DIR)
            continue;

        if (0 == strncmp(dirent->d_name, arg.c_str(), arg.length()))
        {
            Canal::StringStream ss;
            if (!defaultDirPath)
                ss << dirPath;

            ss << dirent->d_name;
            result.push_back(ss.str());
        }
    }

    if (result.size() == 1)
    {
        dirPath = result[0];
        DIR *dir = opendir(dirPath.c_str());
        struct dirent *dirent;
        for (dirent = readdir(dir);
             dirent != NULL;
             dirent = readdir(dir))
        {
            if (!dirent || !dirent->d_name)
                continue;

            if (dirent->d_type != DT_DIR)
                continue;

            Canal::StringStream ss;
            ss << dirPath << "/" << dirent->d_name;
            result.push_back(ss.str());
        }
    }

    return result;
}

void
CommandCd::run(const std::vector<std::string> &args)
{
    if (args.size() > 2)
    {
        llvm::outs() << "Too many parameters.\n";
        return;
    }
    else if (args.size() < 2)
    {
        llvm::outs() << "Argument required (new working directory).\n";
        return;
    }

    wordexp_t exp_result;
    wordexp(args[1].c_str(), &exp_result, 0);

    int save_errno = errno;
    if (-1 == chdir(exp_result.we_wordv[0]))
    {
        llvm::outs() << "Failed to change directory: "
                     << strerror(errno) << "\n";

        errno = save_errno;
    }

    wordfree(&exp_result);
}
