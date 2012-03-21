/*
  A test program doing various string operations using pointer
  arithmetics and libc functions.

  $ clang string_example.c -S -emit-llvm -g
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
  char *path1 = "/etc/hosts";
  FILE *fp = fopen(path1, "r");

  /* return value ignored but should be 0 */
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *contents_heap = malloc(size + 1);
  size_t blocks = fread(contents_heap, size, 1, fp);
  contents_heap[size] = '\0';
  assert(blocks == 1);

  char contents_stack[size + 1];
  fseek(fp, 0, SEEK_SET);
  blocks = fread(contents_stack, size, 1, fp);
  contents_stack[size] = '\0';
  assert(blocks == 1);

  fclose(fp);

  if (strcmp(contents_heap, contents_stack) != 0)
    {
      fprintf(stdout, "stack and heap file contents differ\n");
      fprintf(stdout, "file size: %lu\n", size);
      fprintf(stdout, "heap length: %lu\n", strlen(contents_heap));
      fprintf(stdout, "stack length: %lu\n", strlen(contents_stack));
    }
  else
    printf("stack and heap files are the same\n");

  free(contents_heap);

  char *path2 = malloc(strlen(path1) + strlen(".second") + 1);
  strcpy(path2, path1);
  strcpy(path2 + strlen(path1), ".second");
  printf("second path is %s\n", path2);

  fp = fopen(path2, "r");
  if (fp)
    fclose(fp);

  free(path2);

  return 0;
}
