#ifndef CANAL_WRAPPER_H
#define CANAL_WRAPPER_H

#include "Prereq.h"

class WrapperGcc
{
    int mArgCount;
    char **mArgVector;

public:
    WrapperGcc(int argCount, char **argVector);

    int run();

private:
    int runOriginalTool();
    void runLlvm();
};

#endif // CANAL_WRAPPER_H
