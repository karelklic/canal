#include "CommandShell.h"
#include <sys/types.h>
#include <sys/wait.h>
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

static pid_t
safe_waitpid(pid_t pid, int *wstat, int options)
{
	pid_t r;
	do {
            r = waitpid(pid, wstat, options);
	} while (r == -1 && errno == EINTR);
	return r;
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
        for (std::vector<std::string>::const_iterator it = args.begin() + 1; it != args.end(); ++it)
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

    int save_errno = errno;
    pid_t pid = vfork();
    if (pid < 0)
    {
        printf("Failed to vfork: %s\n", strerror(errno));
        errno = save_errno;
        free(argv);
        return;
    }

    if (pid == 0)
    {
        execvp(argv[0], argv);
        _exit(127);
    }

    safe_waitpid(pid, NULL, 0);
    free(argv);
}
