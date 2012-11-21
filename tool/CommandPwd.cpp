#include "CommandPwd.h"
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>

CommandPwd::CommandPwd(Commands &commands)
    : Command("pwd",
              "",
              "Print working directory",
              "Print working directory.",
              commands)
{
}

void
CommandPwd::run(const std::vector<std::string> &args)
{
    if (args.size() > 1)
    {
        llvm::outs() << "Too many arguments.\n";
        return;
    }

    int save_errno = errno;
    char *cwd = get_current_dir_name();
    if (!cwd)
    {
        llvm::outs() << "Failed to get working directory: " << strerror(errno) << "\n";
        errno = save_errno;
    }

    llvm::outs() << "Working directory " << cwd << ".\n";
    free(cwd);
}
