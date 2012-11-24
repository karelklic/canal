#ifndef LIBCANAL_POINTER_TARGET_H
#define LIBCANAL_POINTER_TARGET_H

#include "Prereq.h"
#include <string>
#include <cstring>
#include <vector>

namespace Canal {

class State;
class SlotTracker;
class Domain;
class Environment;

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
class Target
{
public:
    enum Type {
        Constant,
        Block,
        Function
    };

    const Environment &mEnvironment;

    /// Type of the target.
    Type mType;

    /// Valid when the target represents a memory block or function.
    /// For a memory block, this is a key to either State::mGlobalBlocks or
    /// State::mFunctionBlocks.  The referenced llvm::Value instance is
    /// owned by the LLVM framework and not by this class.
    const llvm::Value *mTarget;

    /// Array or struct offsets in the GetElementPtr style.  This
    /// class owns the memory.  If the vector is empty, mNumericOffset
    /// is considered.  mElementOffsets and mNumericOffset are never
    /// used together.
    std::vector<Domain*> mElementOffsets;

    /// A numeric offset.  The value represents a number of bytes from
    /// the beginning of the object block.  This class owns the
    /// memory.  It might be NULL instead of 0.
    Domain *mNumericOffset;

public:
    /// Standard constructor.
    /// @param type
    ///   Type of the referenced memory.
    /// @param target
    ///   Represents the target memory block or function.  If type is
    ///   Constant, it must be NULL.  If type is Function, it must be
    ///   a pointer to a function.  Otherwise, it must be a valid
    ///   pointer to an instruction.  The instruction pointer is a key
    ///   to State::mFunctionBlocks or State::mGlobalBlocks.
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
    Target(const Environment &environment,
           Type type,
           const llvm::Value *target,
           const std::vector<Domain*> &elementOffsets,
           Domain *numericOffset);

    /// Copy constructor.
    Target(const Target &target);

    /// Standard destructor.
    ~Target();

    bool operator==(const Target &target) const;
    bool operator!=(const Target &target) const;

    /// Merge another target into this one.
    void merge(const Target &target);

    /// Get memory usage (used byte count) of this value.
    size_t memoryUsage() const;

    /// Get a string representation of the target.
    std::string toString(SlotTracker &slotTracker) const;

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments.  Do not implement!
    Target &operator=(const Target &value);
};

} // namespace Pointer
} // namespace Canal

#endif // LIBCANAL_POINTER_TARGET_H
