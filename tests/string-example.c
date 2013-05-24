#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv)
{
    char path[12] = "/var/";
    char suffix[3] = "ab";
    char prefix[7] = "/root/";

    int max = 4;
    for (int i = 0; i < max; i++)
    {
        char *tmp = NULL;
        if (i < 2)
            tmp = strcat(path, suffix);
        else
            tmp = strcat(prefix, path);

        printf("path: %s\n", tmp);
    }

    return EXIT_SUCCESS;
}

