#include "Utils.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Instruction.h>
#include <errno.h>
#include <cstdlib>
#include <climits>
#include <sys/wait.h>

bool
askYesNo(const char *question)
{
    printf("%s [y/n]: ", question);
    fflush(NULL);

    char answer[16];
    if (!fgets(answer, sizeof(answer), stdin))
        return false;

    // Use strncmp here because the answer might contain a newline as
    // the last char.
    int yes = strncmp(answer, "y", 1);
    int no = strncmp(answer, "n", 1);
    return (yes == 0 || no == 0) ? yes == 0 : askYesNo(question);
}

unsigned
stringToUnsigned(const char *str, bool &success)
{
    int save_errno = errno;
    errno = 0;
    char *e;
    unsigned long r = strtoul(str, &e, 10);
    success = !(errno || str == e || *e != '\0' || r > UINT_MAX);
    errno = save_errno;
    return r;
}

int
stringToPositiveInt(const char *str, bool &success)
{
    unsigned r = stringToUnsigned(str, success);
    if (r > (unsigned)INT_MAX)
        success = false;
    return r;
}

int
stringToInt(const char *str, bool &success)
{
    if (*str != '-')
        return stringToPositiveInt(str, success);

    unsigned r = stringToUnsigned(str + 1, success);
    if (r > (unsigned)INT_MAX + 1)
        success = false;
    return -(int)r;
}

pid_t
safeWaitPid(pid_t pid, int *wstat, int options)
{
    pid_t r;
    do {
        r = waitpid(pid, wstat, options);
    } while (r == -1 && errno == EINTR);
    return r;
}
