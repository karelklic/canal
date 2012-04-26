#ifndef CANAL_UTILS_H
#define CANAL_UTILS_H

namespace llvm {
    class Instruction;
}

// Asks a [y/n] question on stdin/stdout.
// Returns true if the answer is yes, false otherwise.
bool askYesNo(const char *question);

void print(const llvm::Instruction &instruction);

#endif // CANAL_UTILS_H
