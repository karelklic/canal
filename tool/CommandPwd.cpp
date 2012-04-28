#include "CommandPwd.h"
#include <unistd.h>
#include <errno.h>
#include <cstdio>
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
        puts("Too many arguments.");
        return;
    }

    int save_errno = errno;
    char *cwd = get_current_dir_name();
    if (!cwd)
    {
        printf("Failed to get working directory: %s\n", strerror(errno));
        errno = save_errno;
    }

    printf("Working directory %s.\n", cwd);
    free(cwd);
}
