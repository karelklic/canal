#ifndef LIBCANAL_MEMORY_UTILS_H
#define LIBCANAL_MEMORY_UTILS_H

#include "Prereq.h"

namespace Canal {
namespace Memory {
namespace Utils {

/// @param offset
///   Offset to the target block.  It might be NULL when the
///   pointer points to the beginning of the block.  The pointer
///   becomes the owner of the provided offset.
void addTarget(Domain &pointer,
               const llvm::Value &target,
               Domain *offset);

void addTarget(Domain &pointer,
               const llvm::Function &target);

/// @brief
///   Converts GetElementPtr-style offsets to a byte offset
///
/// GetElementPtr-style offsets is a list of indices to a composite
/// object.  For example, indices 1 0 2 point to the variable C2 in
/// the following structure:
///
/// struct Example {
///   int A0;       // 0
///   struct A1 {   // 1
///     struct B0 { // 1 0
///       int C0;   // 1 0 0
///       int C1;   // 1 0 1
///       int C2;   // 1 0 2
///     }
///     int B1;     // 1 1
///   }
/// }
///
///
/// Element offsets are abstract 64-bit integer values.  It means each
/// offset might represent multiple concrete integer offsets.
Domain *getByteOffset(std::vector<const Domain*>::const_iterator elementsBegin,
                      std::vector<const Domain*>::const_iterator elementsEnd,
                      const llvm::Type &type,
                      const Environment &environment);

// Does a value represent a local (stack based) or global place.
bool isGlobal(const llvm::Value &place);

} // namespace Utils
} // namespace Memory
} // namespace Canal

#endif // LIBCANAL_MEMORY_UTILS_H
