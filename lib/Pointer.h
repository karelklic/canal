#ifndef LIBCANAL_POINTER_H
#define LIBCANAL_POINTER_H

#include "Domain.h"
#include "PointerTarget.h"
#include <map>
#include <typeinfo>

namespace Canal {

class Constructors;

namespace Pointer {

/// llvm::Value represents a position in the program.  It points to the
/// instruction where the target was assigned/stored to the pointer.
/// The idea is that the targets created on the same place of the code
/// are merged together.  It is a way how targets can be ordered
/// easily.
typedef std::map<const llvm::Value*, Target*> PlaceTargetMap;

/// Inclusion-based flow-insensitive abstract pointer.
class Pointer : public Domain
{
public:
    /// llvm::Value represents a position in the program.  It points to
    /// the instruction where the target was assigned/stored to the
    /// pointer.
    PlaceTargetMap mTargets;

    /// Type of the object the pointer is pointing to.  It might be
    /// incompatible with the type of the actual abstract value.
    /// Conversion is needed during store and load operations in such a
    /// case.
    //
    /// The type object is owned by the LLVM framework.
    const llvm::Type &mType;

public:
    /// Standard constructor.
    Pointer(const Environment &environment,
            const llvm::Type &type);

    /// Copy constructor.
    Pointer(const Pointer &value);

    /// Copy constructor which changes the pointer type.
    /// Useful for bitcast and getelementptr operations.
    Pointer(const Pointer &value,
            const llvm::Type &newType);

    /// Standard destructor.
    virtual ~Pointer();

    const llvm::Type &getType() const { return mType; }

    /// Add a new target to the pointer.
    /// @param type
    ///   Type of the referenced memory.
    /// @param place
    ///   Place where the pointer target is added.
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
    ///   It might be NULL, which indicates the offset 0.  The newly
    ///   created pointer target becomes the owner of the numerical
    ///   offset when it's provided.
    //
    ///   This parameter is mandatory for pointers of Constant type,
    ///   because it contains the constant.
    void addTarget(Target::Type type,
                   const llvm::Value *place,
                   const llvm::Value *target,
                   const std::vector<Domain*> &offsets,
                   Domain *numericOffset);

    /// Dereference all targets and merge the results into single
    /// abstract value.  The returned value is owned by the caller.
    /// @returns
    ///   It might return NULL.
    Domain *dereferenceAndMerge(const State &state) const;

    /// Creates a copy of this object with a different pointer type.
    Pointer *bitcast(const llvm::Type &type) const;

    /// Creates a copy of this object pointing to subtargets.
    /// @param offsets
    ///   Pointer takes ownership of the values inside the vector.
    ///   The offsets must be converted to 64-bit integers before calling
    ///   getElementPtr!
    Pointer *getElementPtr(const std::vector<Domain*> &offsets,
                           const llvm::Type &type,
                           const Constructors &constructors) const;

    void store(const Domain &value, State &state) const;

    /// Does this pointer point to single target?
    bool isSingleTarget() const;

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Pointer &operator=(const Pointer &value);

public: // Implementation of Domain.
    /// Implementation of Domain::clone().
    /// Covariant return type -- it really overrides Domain::clone().
    virtual Pointer *clone() const;
    /// Implementation of Domain::operator==().
    virtual bool operator==(const Domain &value) const;
    /// Implementation of Domain::merge().
    virtual void merge(const Domain &value);
    /// Implementation of Domain::memoryUsage().
    virtual size_t memoryUsage() const;
    /// Implementation of Domain::toString().
    virtual std::string toString() const;
    /// Implementation of Domain::setZero().
    virtual void setZero(const llvm::Value *place);
};
DONT_USE_COW(Pointer);
} // namespace Pointer
} // namespace Canal

#endif // LIBCANAL_POINTER_H
