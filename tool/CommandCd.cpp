#include "CommandCd.h"
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <cstring>

CommandCd::CommandCd(Commands &commands)
    : Command("cd",
              "",
              "Set working directory to DIR for interpreter",
              "Set working directory to DIR for interpreter.  "
              "Affects module loading.",
              commands)
{
}

void
CommandCd::run(const std::vector<std::string> &args)
{
    if (args.size() > 2)
    {
        puts("Too many parameters.");
        return;
    }
    else if (args.size() < 2)
    {
        puts("Argument required (new working directory).");
        return;
    }

    int save_errno = errno;
    if (-1 == chdir(args[1].c_str()))
    {
        printf("Failed to change directory: %s\n", strerror(errno));
        errno = save_errno;
    }
}
