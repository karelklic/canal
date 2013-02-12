#ifndef LIBCANAL_MEMORY_BLOCK_MAP_H
#define LIBCANAL_MEMORY_BLOCK_MAP_H

#include "MemoryMap.h"
#include "MemoryBlock.h"

namespace Canal {
namespace Memory {

/// The key represents a place in the program where the memory
/// block is allocated.  Usually, this is a llvm::Instruction --
/// either alloca, or a call to the malloc function.
///
/// The values are owned by this class, so they are deleted in the
/// state destructor.
class BlockMap : public Map<Block>
{
public:
    void joinHeap(const BlockMap &map);
    void joinStack(const BlockMap &map);
};

} // namespace Memory
} // namespace Canal

#endif // LIBCANAL_MEMORY_BLOCK_MAP_H
