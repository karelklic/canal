#ifndef LIBCANAL_CONSTRUCTORS_H
#define LIBCANAL_CONSTRUCTORS_H

namespace llvm {
class Type;
class Constant;
class ConstantExpr;
struct fltSemantics;
} // namespace llvm

namespace Canal {

class Domain;
class Environment;
class State;

class Constructors
{
protected:
    const Environment &mEnvironment;

public:
    Constructors(const Environment &environment);

    Domain *create(const llvm::Type &type) const;

    /// @param state
    ///   State is used only for constant expressions such as
    ///   getelementptr and bitcast.  For other types of constants it
    ///   might be NULL.
    /// @param
    ///   Returns a newly allocated value or NULL.  Caller takes
    ///   ownership of the returned value.
    Domain *create(const llvm::Constant &value,
                   const State *state) const;

    static const llvm::fltSemantics *
        getFloatingPointSemantics(const llvm::Type &type);

protected:
    Domain *createGetElementPtr(const llvm::ConstantExpr &value,
                                const Domain &variable) const;

    Domain *createBitCast(const llvm::ConstantExpr &value,
                          const Domain &variable) const;
};

} // namespace Canal

#endif // LIBCANAL_CONSTRUCTORS_H
