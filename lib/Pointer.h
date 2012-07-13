#ifndef LIBCANAL_POINTER_H
#define LIBCANAL_POINTER_H

#include "Value.h"
#include <map>

namespace llvm {
    class Value;
}

namespace Canal {

class SlotTracker;

namespace Pointer {

// Pointer target -- where the pointer points to.
// Pointer can:
//  - be set to a fixed constant (memory area), such as 0xbaadfood
//  - point to a heap object (global block)
//  - point to a stack object (alloca, function block)
// Pointer can point to some offset in an array.
//
// TODO: Pointers to structure members.  Pointers to functions.
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
    std::string toString(const State *state, SlotTracker &slotTracker) const;

    Value &dereference(const State &state) const;

public:
    Type mType;

    // Valid when the target represents an anonymous memory block.
    // This is a key to either State::mGlobalBlocks or
    // State::mFunctionBlocks.
    const llvm::Value *mInstruction;
    // A specific constant.
    size_t mConstant;

    // Array or struct offsets in the GetElementPtr style.
    std::vector<Value*> mOffsets;
};

// llvm::Value represents a position in the program.  It points to
// the instruction where the target was assigned/stored to the
// pointer.
typedef std::map<const llvm::Value*, Target> PlaceTargetMap;

// Inclusion-based flow-insensitive abstract pointer.
class InclusionBased : public Value
{
public:
    InclusionBased(const llvm::Module &module);

    // Implementation of Value::clone().
    // Covariant return type -- it really overrides Value::clone().
    virtual InclusionBased* clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;

    void addConstantTarget(const llvm::Value *instruction, size_t constant);

    // @param instruction
    //  Place where the pointer target is added.
    // @param target
    //   Represents an anonymous memory block.  This is a key to either
    //   State::mGlobalBlocks or State::mFunctionBlocks.
    // @param arrayOffset
    //   If the pointer is provided, this class takes ownership of it.
    void addMemoryTarget(const llvm::Value *instruction, const llvm::Value *target, Value *arrayOffset = NULL);

    const PlaceTargetMap &getTargets() const { return mTargets; }

protected:
    const llvm::Module &mModule;

    // llvm::Value represents a position in the program.  It points to
    // the instruction where the target was assigned/stored to the
    // pointer.
    PlaceTargetMap mTargets;
};

} // namespace Pointer
} // namespace Canal

#endif // LIBCANAL_POINTER_H
