#ifndef LIBCANAL_OPERATIONS_CALLBACK_H
#define LIBCANAL_OPERATIONS_CALLBACK_H

#include <vector>

namespace llvm {
class Function;
} // namespace llvm

namespace Canal {

class Domain;

class OperationsCallback
{
public:
    virtual Domain *onFunctionCall(const llvm::Function &function,
                                   const std::vector<Domain*> &arguments) = 0;
};

} // namespace Canal

#endif // LIBCANAL_OPERATIONS_CALLBACK_H
