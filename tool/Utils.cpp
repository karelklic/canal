#include "Utils.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Instruction.h>

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

void
print(const llvm::Instruction &instruction)
{
    std::string s;
    llvm::raw_string_ostream os(s);
    os << instruction;
    puts(s.c_str());
}
