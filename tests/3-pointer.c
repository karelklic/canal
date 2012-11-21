struct munger
{
  int f1;
  int f2;
};

void munge(struct munger *p)
{
  p[0].f1 = p[1].f1 + p[2].f2;
}

int main(void)
{
    struct munger array[3];
    munge(array);
    return 0;
}
