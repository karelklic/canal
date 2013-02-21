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
    void join(const BlockMap &map)
    {
        Map<Block>::join(map);
    }

    void join(const BlockMap &map,
              Block::MemoryType memoryType);

    // Merge function blocks that do not belong to current function.
    void joinForeignStack(const BlockMap &map,
                          const llvm::Function &currentFunction);

    void findNames(const std::string &prefix,
                   Block::MemoryType memoryType,
                   SlotTracker &slotTracker,
                   std::vector<std::string> &result) const;
};

} // namespace Memory
} // namespace Canal

#endif // LIBCANAL_MEMORY_BLOCK_MAP_H
