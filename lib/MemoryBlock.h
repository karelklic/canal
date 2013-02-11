#ifndef LIBCANAL_MEMORY_BLOCK_H
#define LIBCANAL_MEMORY_BLOCK_H

#include "SharedDataPointer.h"

namespace Canal {
namespace Memory {

/// Abstracts a continuous memory block.
class Block : public SharedData
{
    mutable Domain *mMainValue;

public:
    enum MemoryType {
        StackMemoryType,
        HeapMemoryType
    } mMemoryType;

public:
    /// @param mainValue
    ///   The block takes ownership of the provided abstract value.
    Block(MemoryType memoryType, Domain *mainValue);

    Block(const Block &block);

    ~Block();

    bool operator==(const Block &value) const;

    bool operator!=(const Block &value) const
    {
        return !operator==(value);
    }

    bool operator<(const Block &value) const;

    /// For SharedDataPtr.
    Block *clone() const
    {
        return new Block(*this);
    }

    size_t memoryUsage() const;

    std::string toString() const;

    void join(const Block &block);

    /// Merges all cells to the main value and returns it.
    Domain &getMainValue() const
    {
        return *mMainValue;
    }

    /// @param offset
    ///   Offset in bytes.
    Domain *load(const llvm::Type &type,
                 const Domain &offset) const;

    /// @param offset
    ///   Offset in bytes.
    /// @param isSingleTarget
    ///   Helps to decide whether to set the value or join it to the
    ///   existing value.
    void store(const Domain &value,
               const Domain &offset,
               bool isSingleTarget);
};

} // namespace Memory
} // namespace Canal

#endif // LIBCANAL_MEMORY_BLOCK_H
