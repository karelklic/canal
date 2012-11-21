#ifndef CANAL_UTILS_H
#define CANAL_UTILS_H

#include "Prereq.h"
#include <sys/types.h>

// Asks a [y/n] question on stdin/stdout.
// Returns true if the answer is yes, false otherwise.
bool askYesNo(const char *question);

unsigned stringToUnsigned(const char *str, bool &success);
int stringToPositiveInt(const char *str, bool &success);
int stringToInt(const char *str, bool &success);

pid_t safeWaitPid(pid_t pid, int *wstat, int options);

#endif // CANAL_UTILS_H
