#ifndef LIBCANAL_POINTER_UTILS_H
#define LIBCANAL_POINTER_UTILS_H

#include "PointerTarget.h"

namespace Canal {
namespace Pointer {
namespace Utils {

void addTarget(Domain &pointer,
               Target::Type type,
               const llvm::Value *place,
               const llvm::Value *target,
               const std::vector<Domain*> &offsets,
               Domain *numericOffset);

} // namespace Utils
} // namespace Pointer
} // namespace Canal

#endif // LIBCANAL_POINTER_UTILS_H
