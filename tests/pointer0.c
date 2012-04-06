#include <stddef.h>

void swap()
{
    int x = 0, y = 1, *px = &x, *py = &y;
    int temp;
    temp = *px; *px = *py; *py = temp;
}

void array(int i)
{
    int a[3] = {0, 1, 2};
    int *j = &a[i];
    int *k = j;
    *k = 218;
}

void strcpy_simple(char *s, char *t)
{
    while ((*s++ = *t++)) ;
}

void append(char *s)
{
    *s++ = 'a';
    *s++ = 'b';
    *s++ = '\0';
}

int main(int argc, char **argv)
{
    int i = 3, ii = 6, *j, k[i];
    j = &i;
    *j = 4;
    j = &ii;
    *j = 5;
    int *l;
    l = j;
    j = NULL;
    *l = 1;
    k[*l] = 0;
    return 0;
}
