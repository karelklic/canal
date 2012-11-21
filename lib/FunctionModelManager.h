#ifndef LIBCANAL_FUNCTION_MODEL_MANAGER_H
#define LIBCANAL_FUNCTION_MODEL_MANAGER_H

#include "Prereq.h"

namespace Canal {

class FunctionModelMAnager
{
public:
    bool canHandle(llvm::Function *function,
                   bool implementationAvailable);

    void handle(llvm::Function *function,
                State &state);
};

} // namespace Canal

#endif // LIBCANAL_FUNCTION_MODEL_MANAGER_H
