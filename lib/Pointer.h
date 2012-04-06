#ifndef CANAL_POINTER_H
#define CANAL_POINTER_H

#include "Value.h"
#include <map>

namespace llvm {
    class Value;
}

namespace Canal {
namespace Pointer {

// Pointer target -- where the pointer points to.
// Pointer can:
//  - be set to a fixed constant (memory area), such as 0xbaadfood
//  - point to a global variable (=globals memory, global variable)
//  - point to a heap object (global block)
//  - point to a stack variable (function variable)
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
        GlobalVariable,
        GlobalBlock,
        FunctionVariable,
        FunctionBlock
    };

    // Initializes the target to the Uninitialized type.
    Target();

    bool operator==(const Target &target) const;
    bool operator!=(const Target &target) const;

    Type getType() const { return mType; }

    // Set the offset to the memory block.
    void setBlockOffset(size_t offset, Type type);
    // Get the offset of the memory block where this target points to.
    size_t getBlockOffset() const;

    // Set the target to a global or function variable.
    void setVariable(const llvm::Value *variable, Type type);
    // Get the global or function variable this target points to.
    const llvm::Value *getVariable() const;

    // Set the target to a constant memory address, such as NULL or
    // 0xbaadfood.
    void setConstant(size_t constant);
    // Get a constant value the target points to, such as NULL or 0xbaadfood.
    size_t getConstant() const;

    bool isArrayOffset() const { return mIsArrayOffset; }
    // TODO: Use Integer conteiner instead of min-max.
    size_t minArrayOffset() const { return mMinArrayOffset; }
    size_t maxArrayOffset() const { return mMaxArrayOffset; }
    void setArrayOffset(size_t minOffset, size_t maxOffset);

    // Dereference the pointer on a state.
    //
    // If the Target represents a constant or uninitialized pointer,
    // NULL is returned.
    Value *dereference(State &state) const;
    const Value *dereference(const State &state) const;

protected:
    Type mType;

    union {
        // Valid when the target represents a variable.
        const llvm::Value *mVariable;
        // Valid when the target represents an anonymous memory block.
        // This is an offset to OperationalState::mGlobalBlocks or
        // OperationalState::mFunctionBlocks.
        size_t mOffset;
        // A specific constant.
        size_t mConstant;
    };

    // If the array offset is valid.
    bool mIsArrayOffset;
    // Minimal array offset.
    size_t mMinArrayOffset;
    // (size_t)-1 means infinity (all array items).
    size_t mMaxArrayOffset;
};

// Inclusion-based flow-insensitive abstract pointer.
class InclusionBased : public Value, public StateOwner
{
public:
    // llvm::Value represents a position in the program. It points to
    // the instruction of origin.
    std::map<const llvm::Value*, Target> mTargets;

public:
    // Implementation of Value::clone().
    // Covariant return type -- it really overrides Value::clone().
    virtual InclusionBased* clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::limitmemoryUsage().
    virtual bool limitMemoryUsage(size_t size);
    // Implementation of Value::accuracy().
    virtual float accuracy() const;
    // Implementation of Value::isBottom().
    virtual bool isBottom() const;
    // Implementation of Value::setTop().
    virtual void setTop();
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;
};

} // namespace Pointer
} // namespace Canal

#endif
