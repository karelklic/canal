#ifndef CANAL_ABSTRACT_POINTER_H
#define CANAL_ABSTRACT_POINTER_H

#include "AbstractValue.h"
#include <set>

namespace AbstractPointer {

// Pointer target -- where the pointer points.
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
    size_t minArrayOffset() const { return mMinArrayOffset; }
    size_t maxArrayOffset() const { return mMaxArrayOffset; }
    void setArrayOffset(size_t minOffset, size_t maxOffset);

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
    }

    // If the array offset is valid.
    bool mIsArrayOffset;
    // Minimal array offset.
    size_t mMinArrayOffset;
    // (size_t)-1 means infinity (all array items).
    size_t mMaxArrayOffset;
};

// Inclusion-based flow-insensitive abstract pointer.
class InclusionBased : public AbstractValue
{
public:
    std::set<Target> mTargets;

public:
    // Implementation of AbstractValue::clone().
    // Covariant return type -- it really overrides AbstractValue::clone().
    virtual InclusionBased* clone() const;
    // Implementation of AbstractValue::operator==().
    virtual bool operator==(const AbstractValue &value) const;
    // Implementation of AbstractValue::merge().
    virtual void merge(const AbstractValue &value);
    // Implementation of AbstractValue::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of AbstractValue::limitmemoryUsage().
    virtual bool limitMemoryUsage(size_t size);
    // Implementation of AbstractValue::accuracy().
    virtual float accuracy() const;
    // Implementation of AbstractValue::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;
};

} // namespace AbstractPointer

#endif
