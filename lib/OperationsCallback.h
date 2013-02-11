#ifndef LIBCANAL_OPERATIONS_CALLBACK_H
#define LIBCANAL_OPERATIONS_CALLBACK_H

#include "Prereq.h"
#include <vector>

namespace Canal {

class OperationsCallback
{
public:
    virtual ~OperationsCallback() {}

    /// @param callState
    ///   Contains function arguments and referenced memory blocks.
    /// @param resultState
    ///   A state where the changes caused by the function can be
    ///   merged.
    /// @param resultPlace
    ///   A place in the resultState where the returned value should
    ///   be merged.
    virtual void onFunctionCall(const llvm::Function &function,
                                const Memory::State &callState,
                                Memory::State &resultState,
                                const llvm::Value &resultPlace) = 0;
};

} // namespace Canal

#endif // LIBCANAL_OPERATIONS_CALLBACK_H
