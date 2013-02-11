#ifndef LIBCANAL_MEMORY_POINTER_H
#define LIBCANAL_MEMORY_POINTER_H

#include "Domain.h"
#include <map>
#include <set>

namespace Canal {
namespace Memory {

/// Abstract pointer.
class Pointer : public Domain
{
    /// @brief
    ///   A flag indicating that the pointer can point to all parts of
    ///   memory.
    ///
    /// Store operation to such a topped pointer will be ignored.
    bool mTop;

    /// @brief
    ///   A map of possible target blocks.
    ///
    /// The key represents a memory block.  It can be used to find the
    /// block's abstract value in State::mGlobalBlocks or
    /// State::mFunctionBlocks.
    ///
    /// The value represents a numerical offset to the block.  It must
    /// be a 64-bit integer type.  It is never NULL.
    std::map<const llvm::Value*, Domain*> mBlockTargets;

    /// A NULL or other numeric value (e.g. 0xBAADFEED) can be
    /// assigned to a pointer.  Such a numeric values are represented
    /// by a numeric abstract value.  mNumericOffset might be NULL if
    /// no numeric value was assigned to the pointer.
    Domain *mNumericOffset;

    /// If the pointer points to a function, here is a list of
    /// potential targets.
    std::set<const llvm::Function*> mFunctionTargets;

    /// Type of the pointer.  The referenced type might be
    /// incompatible with the type of the actual abstract value.
    /// Conversion is needed during store and load operations in such
    /// a case.
    //
    /// The type object is owned by the LLVM framework.
    const llvm::PointerType &mType;

public:
    /// Standard constructor.
    Pointer(const Environment &environment,
            const llvm::PointerType &type);

    /// Copy constructor.
    Pointer(const Pointer &value);

    /// Copy constructor which changes the pointer type.
    /// Useful for bitcast and getelementptr operations.
    Pointer(const Pointer &value,
            const llvm::PointerType &newType);

    /// @brief
    ///   Standard destructor.
    virtual ~Pointer();

    /// @brief
    ///   Add a numerical target to the pointer.
    void addTarget(const Domain &numericOffset);

    /// @brief
    ///   Add a block target to the pointer.
    /// @param target
    ///   Represents the target memory block. It must be a valid
    ///   referenece to an instruction.  This is a key to
    ///   State::mFunctionBlocks or State::mGlobalBlocks.
    /// @param offset
    ///   Offset to the target block.  It might be NULL when the
    ///   pointer points to the beginning of the block.  The pointer
    ///   becomes the owner of the provided offset.
    void addTarget(const llvm::Value &target, Domain *offset);

    /// Add a function target to the pointer.
    void addTarget(const llvm::Function &function);

    /// Dereference all targets and merge the results into single
    /// abstract value.  The returned value is owned by the caller.
    /// @returns
    ///   It might return NULL in the case when the pointer is in the
    ///   bottom state (no targets).
    Domain *dereferenceAndMerge(const State &state) const;

    /// Creates a copy of this pointer adjusted by an offset.
    Pointer *withOffset(const Domain &offset,
                        const llvm::PointerType &type) const;

    void store(const Domain &value, State &state) const;

    /// Does this pointer point to single target?
    bool isConstant() const;

    void widen(const Pointer &second);

    static bool classof(const Domain *value)
    {
        return value->getKind() == PointerKind;
    }

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Pointer &operator=(const Pointer &value);

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Pointer *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual Pointer &join(const Domain &value);

    virtual Pointer &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    /// Covariant return type.
    virtual const llvm::PointerType &getValueType() const { return mType; }
};

} // namespace Memory
} // namespace Canal

#endif // LIBCANAL_MEMORY_POINTER_H
