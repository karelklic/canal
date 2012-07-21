#ifndef LIBCANAL_POINTER_H
#define LIBCANAL_POINTER_H

#include "Value.h"
#include "PointerTarget.h"
#include <map>

namespace llvm {
    class Value;
}

namespace Canal {
namespace Pointer {

// llvm::Value represents a position in the program.  It points to
// the instruction where the target was assigned/stored to the
// pointer.
typedef std::map<const llvm::Value*, Target> PlaceTargetMap;

// Inclusion-based flow-insensitive abstract pointer.
class InclusionBased : public Value
{
public:
    // Used in toString.
    const llvm::Module &mModule;

    // llvm::Value represents a position in the program.  It points to
    // the instruction where the target was assigned/stored to the
    // pointer.
    PlaceTargetMap mTargets;

    const llvm::Type *mBitcastFrom;
    const llvm::Type *mBitcastTo;

public:
    InclusionBased(const llvm::Module &module);

    void addConstantTarget(const llvm::Value *instruction, size_t constant);

    // @param instruction
    //  Place where the pointer target is added.
    // @param target
    //   Represents an anonymous memory block.  This is a key to either
    //   State::mGlobalBlocks or State::mFunctionBlocks.
    void addMemoryTarget(const llvm::Value *instruction,
                         const llvm::Value *target);

public: // Implementation of Value.
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
};

} // namespace Pointer
} // namespace Canal

#endif // LIBCANAL_POINTER_H
