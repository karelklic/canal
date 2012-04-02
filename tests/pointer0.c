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

void strcpy(char *s, char *t)
{
    while (*s++ = *t++) ;
}

void strcpy_t(char *s)
{
    *s++ = ’a’;
    *s++ = ’b’;
    *s++ = ’\0’;
}

int main(int argc, char **argv)
{
    int i = 3, ii = 6, *j;
    j = &i;
    *j = 4;
    j = &ii;
    *j = 5;
    int *k;
    k = j;
    j = NULL;
    *k = 6;
    return 0;
}
