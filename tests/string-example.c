#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv)
{
    char path[12] = "/var/";
    char suffix[3] = "ab";
    char prefix[7] = "/root/";

    char *tmp1 = NULL, *tmp2 = NULL;
    int max = 2;
    for (int i = 0; i < max; i++)
    {
        if (i < 1)
            tmp1 = strcat(path, suffix);
        else
            tmp1 = strcat(prefix, path);
    }

    char *result = tmp1;

    return EXIT_SUCCESS;
}

