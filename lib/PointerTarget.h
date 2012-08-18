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

/// Pointer target -- where the pointer points to.
/// Pointer can:
///  - be set to a fixed constant (memory area), such as 0xbaadfood
///  - point to a heap object (global block)
///  - point to a stack object (alloca, function block)
/// Pointer can point to some offset in an array.
//
/// TODO: Pointers to functions.
class Target
{
public:
    enum Type {
        Constant,
        FunctionBlock,
        FunctionVariable,
        GlobalBlock,
        GlobalVariable,
    };

    /// Standard constructor.
    /// @param type
    ///   Type of the referenced memory.
    /// @param target
    ///   Represents the target memory block.  If type is Constant, it
    ///   must be NULL.  Otherwise, it must be a valid pointer to an
    ///   instruction.  This is a key to State::mFunctionBlocks,
    ///   State::mFunctionVariables, State::mGlobalBlocks, or
    ///   State::mGlobalVariables, depending on the type.
    /// @param offsets
    ///   Offsets in the getelementptr style.  The provided vector
    ///   might be empty.  The newly created pointer target becomes the
    ///   owner of the objects in the vector.
    /// @param numericOffset
    ///   Numerical offset that is used in addition to the
    ///   getelementptr style offset and after they have been applied.
    ///   It might be NULL, which indicates the offset 0.  The target
    ///   becomes the owner of the numerical offset when it's provided.
    //
    ///   This parameter is mandatory for pointers of Constant type,
    ///   because it contains the constant.
    Target(Type type,
           const llvm::Value *target,
           const std::vector<Value*> &offsets,
           Value *numericOffset);

    /// Copy constructor.
    Target(const Target &target);

    ~Target();

    bool operator==(const Target &target) const;
    bool operator!=(const Target &target) const;

    /// Merge another target into this one.
    void merge(const Target &target);

    /// Get memory usage (used byte count) of this value.
    size_t memoryUsage() const;

    /// Get a string representation of the target.
    std::string toString(SlotTracker &slotTracker) const;

    /// Dereference the target in a certain state.  Dereferencing might
    /// result in multiple Values being returned due to the nature of
    /// mOffsets (offsets might include integer ranges).  The returned
    /// pointers point to the memory owned by State and its abstract
    /// domains -- caller must not release the memory.
    std::vector<Value*> dereference(const State &state) const;

public:
    /// Type of the target.
    Type mType;

    /// Valid when the target represents an anonymous memory block.
    /// This is a key to either State::mGlobalBlocks or
    /// State::mFunctionBlocks.  The referenced llvm::Value instance is
    /// owned by the LLVM framework and not by this class.
    const llvm::Value *mInstruction;

    /// Array or struct offsets in the GetElementPtr style.
    /// This class owns the memory.
    std::vector<Value*> mOffsets;

    /// An additional numeric offset on the top of mOffsets.  The value
    /// represents a number of bytes.  This class owns the memory.  It
    /// might be NULL instead of 0.
    Value *mNumericOffset;
};

} // namespace Pointer
} // namespace Canal

#endif // LIBCANAL_POINTER_TARGET_H
