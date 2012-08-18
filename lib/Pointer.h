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

/// llvm::Value represents a position in the program.  It points to the
/// instruction where the target was assigned/stored to the pointer.
/// The idea is that the targets created on the same place of the code
/// are merged together.  It is a way how targets can be ordered
/// easily.
typedef std::map<const llvm::Value*, Target*> PlaceTargetMap;

/// Inclusion-based flow-insensitive abstract pointer.
class InclusionBased : public Value
{
private:
    /// Used in toString.
    const llvm::Module &mModule;

    /// llvm::Value represents a position in the program.  It points to
    /// the instruction where the target was assigned/stored to the
    /// pointer.
    PlaceTargetMap mTargets;

    /// Type of the object the pointer is pointing to.  It might be
    /// incompatible with the type of the actual abstract value.
    /// Conversion is needed during store and load operations in such a
    /// case.
    //
    /// The type object is owned by LLVM and not deleted by the
    /// InclusionBased class.
    const llvm::Type *mType;

public:
    /// Standard constructor.
    InclusionBased(const llvm::Module &module,
                   const llvm::Type *type);

    /// Copy constructor.
    InclusionBased(const InclusionBased &second);

    /// Standard destructor.
    virtual ~InclusionBased();

    /// Add a new target to the pointer.
    /// @param type
    ///   Type of the referenced memory.
    /// @param instruction
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
                   const llvm::Value *instruction,
                   const llvm::Value *target,
                   const std::vector<Value*> &offsets,
                   Value *numericOffset);

    /// Dereference all targets and merge the results into single
    /// abstract value.  The returned value is owned by the caller.
    /// @returns
    ///   It might return NULL.
    Value *dereferenceAndMerge(const State &state) const;

    /// Creates a copy of this object with a different pointer type.
    InclusionBased *bitcast(const llvm::Type *type) const;

    /// Creates a copy of this object pointing to subtargets.
    /// @param offsets
    ///   Pointer takes ownership of the values inside the vector.
    InclusionBased *getElementPtr(const std::vector<Value*> &offsets,
                                  const llvm::Type *type) const;

    void store(const Value &value, State &state);

public: // Implementation of Value.
    /// Implementation of Value::clone().
    /// Covariant return type -- it really overrides Value::clone().
    virtual InclusionBased *clone() const;
    /// Implementation of Value::cloneCleaned().
    /// Covariant return type.
    virtual InclusionBased *cloneCleaned() const;
    /// Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    /// Implementation of Value::merge().
    virtual void merge(const Value &value);
    /// Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    /// Implementation of Value::toString().
    virtual std::string toString() const;
    /// Implementation of Value::matchesString().
    virtual bool matchesString(const std::string &text,
                               std::string &rationale) const;
};

} // namespace Pointer
} // namespace Canal

#endif // LIBCANAL_POINTER_H
