#ifndef CANAL_POINTER_H
#define CANAL_POINTER_H

#include "Value.h"
#include <map>

namespace llvm {
    class Value;
}

namespace Canal {
namespace Pointer {

class InclusionBased;

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
        GlobalBlock,
        FunctionBlock
    };

    // Initializes the target to the Uninitialized type.
    Target();
    Target(const Target &target);
    ~Target();

    bool operator==(const Target &target) const;
    bool operator!=(const Target &target) const;

    // Dereference the pointer on a state.
    //
    // If the Target represents a constant or uninitialized pointer,
    // NULL is returned.
    Value *dereference() const;

    void merge(const Target &target);

public:
    InclusionBased *mParent;
    Type mType;

    union {
        // Valid when the target represents an anonymous memory block.
        // This is an offset to OperationalState::mGlobalBlocks or
        // OperationalState::mFunctionBlocks.
        size_t mOffset;
        // A specific constant.
        size_t mConstant;
    };

    // Pointer to an array if this is not NULL.
    Value *mArrayOffset;
};

// Inclusion-based flow-insensitive abstract pointer.
class InclusionBased : public Value, public StateValue
{
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
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;

    void addConstantTarget(const llvm::Value *instruction, size_t constant);
    void addHeapTarget(const llvm::Value *instruction, size_t offset, Value *arrayOffset = NULL);
    void addStackTarget(const llvm::Value *instruction, size_t offset, Value *arrayOffset = NULL);

protected:
    // llvm::Value represents a position in the program. It points to
    // the instruction of origin.
    std::map<const llvm::Value*, Target> mTargets;
};

} // namespace Pointer
} // namespace Canal

#endif
