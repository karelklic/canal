#include "CommandShell.h"
#include "Utils.h"
#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>

CommandShell::CommandShell(Commands &commands)
    : Command("shell",
              "",
              "Execute the rest of the line as a shell command",
              "Execute the rest of the line as a shell command.\n"
              "With no arguments, run an inferior shell.\n"
              "$PATH is modified to include gcc/ld build wrappers.",
              commands)
{
}

void
CommandShell::run(const std::vector<std::string> &args)
{
    char *shell_program = getenv("SHELL");
    if (!shell_program)
        shell_program = (char*)"/bin/sh";

    char **argv;
    std::string shell_command;
    if (args.size() == 1)
    {
        // Run inferior shell.
        argv = (char**)malloc(2 * sizeof(char*));
        argv[0] = shell_program;
        argv[1] = NULL;
    }
    else
    {
        std::stringstream ss;
        std::vector<std::string>::const_iterator it = args.begin() + 1;
        for (; it != args.end(); ++it)
        {
            if (it != args.begin() + 1)
                ss << " ";
            ss << *it;
        }
        shell_command = ss.str();

        argv = (char**)malloc(4 * sizeof(char*));
        argv[0] = shell_program;
        argv[1] = (char*)"-c";
        argv[2] = (char*)shell_command.c_str();
        argv[3] = NULL;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Failed to fork: %s\n", strerror(errno));
        free(argv);
        return;
    }

    if (pid == 0)
    {
        std::stringstream newPath;
        newPath << DATADIR << "/canal/wrappers";

        char *oldPath = getenv("PATH");
        if (oldPath)
            newPath << ":" << oldPath;

        int result = setenv("PATH", newPath.str().c_str(), /*overwrite=*/1);
        if (result != 0)
            printf("Failed to set PATH: %s\n", strerror(errno));

        execvp(argv[0], argv);
        _exit(127);
    }

    safeWaitPid(pid, NULL, 0);
    free(argv);
}
