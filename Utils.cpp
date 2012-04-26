#include "Utils.h"
#include <cstdio>
#include <cstring>

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
