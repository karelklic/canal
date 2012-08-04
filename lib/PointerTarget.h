#ifndef LIBCANAL_POINTER_TARGET_H
#define LIBCANAL_POINTER_TARGET_H

#include <string>
#include <cstring>
#include <vector>

namespace llvm {
class Value;
} // namespace llvm

namespace Canal {

class State;
class SlotTracker;
class Value;

namespace Integer {
class Container;
} // namespace Integer

namespace Pointer {

// Pointer target -- where the pointer points to.
// Pointer can:
//  - be set to a fixed constant (memory area), such as 0xbaadfood
//  - point to a heap object (global block)
//  - point to a stack object (alloca, function block)
// Pointer can point to some offset in an array.
//
// TODO: Pointers to functions.
class Target
{
public:
    enum Type {
        Uninitialized,
        Constant,
        MemoryBlock
    };

    // Initializes the target to the uninitialized type.
    Target();
    Target(const Target &target);
    ~Target();

    bool operator==(const Target &target) const;
    bool operator!=(const Target &target) const;

    // Merge another target into this one.
    void merge(const Target &target);

    // Get memory usage (used byte count) of this value.
    size_t memoryUsage() const;

    // Get a string representation of the target.
    std::string toString(const State *state, SlotTracker &slotTracker) const;

    // Dereference the target in a certain state.  Dereferencing might
    // result in multiple Values being returned due to the nature of
    // mOffsets (offsets might include integer ranges).  The returned
    // pointers point to the memory owned by State and its abstract
    // domains -- caller must not release the memory.
    std::vector<Value*> dereference(const State &state) const;

public:
    // Type of the target.
    Type mType;

    // Valid when the target represents an anonymous memory block.
    // This is a key to either State::mGlobalBlocks or
    // State::mFunctionBlocks.  The referenced llvm::Value instance is
    // owned by the LLVM framework and not by this class.
    const llvm::Value *mInstruction;

    // A specific constant.
    size_t mConstant;

    // Array or struct offsets in the GetElementPtr style.
    // This class owns the memory.
    std::vector<Value*> mOffsets;

    // An additional, numeric offset on the top of mOffsets.  This
    // class owns the memory.  It might be NULL instead of 0.
    Value *mNumericOffset;
};

} // namespace Pointer
} // namespace Canal

#endif // LIBCANAL_POINTER_TARGET_H
