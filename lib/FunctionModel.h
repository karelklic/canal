#ifndef LIBCANAL_FUNCTION_MODEL_H
#define LIBCANAL_FUNCTION_MODEL_H

#include "Prereq.h"

namespace Canal {

class FunctionModel
{
public:
    bool canHandle(llvm::Function *function,
                   bool implementationAvailable);

    void handle(llvm::Function *function,
                State &state);
};

} // namespace Canal

#endif // LIBCANAL_CALLBACKS_H
