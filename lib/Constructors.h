#ifndef LIBCANAL_CONSTRUCTORS_H
#define LIBCANAL_CONSTRUCTORS_H

#include "Prereq.h"
#include <inttypes.h>
#include <vector>

namespace Canal {

class Domain;
class Environment;
class State;

class Constructors
{
protected:
    const Environment &mEnvironment;

public:
    Constructors(Environment &environment);

    const Environment &getEnvironment() const
    {
        return mEnvironment;
    }

    Domain *create(const llvm::Type &type) const;

    /// @param state
    ///   State is used only for constant expressions such as
    ///   getelementptr and bitcast.  For other types of constants it
    ///   might be NULL.
    /// @returns
    ///   Returns a newly allocated value or NULL.  Caller takes
    ///   ownership of the returned value.
    Domain *create(const llvm::Constant &value,
                   const llvm::Value &place,
                   const State *state) const;

    Domain *createInteger(unsigned bitWidth) const;

    Domain *createInteger(const llvm::APInt &number) const;

    Domain *createFloat(const llvm::fltSemantics &semantics) const;

    Domain *createFloat(const llvm::APFloat &number) const;

    Domain *createArray(const llvm::SequentialType &type) const;

    Domain *createArray(const llvm::SequentialType &type,
                        Domain *size) const;

    Domain *createArray(const llvm::SequentialType &type,
                        const std::vector<Domain*> &values) const;

    Domain *createPointer(const llvm::PointerType &type) const;

    Domain *createStructure(const llvm::StructType &type) const;

    Domain *createStructure(const llvm::StructType &type,
                            const std::vector<Domain*> &members) const;

protected:
    Domain *createConstantExpr(const llvm::ConstantExpr &value,
                               const llvm::Value &place,
                               const State *state) const;

    Domain *createGetElementPtr(const llvm::ConstantExpr &value,
                                const std::vector<const Domain*> &operands,
                                const llvm::Value &place) const;

    Domain *createBitCast(const llvm::ConstantExpr &value,
                          const std::vector<const Domain*> &operands,
                          const llvm::Value &place) const;
};

} // namespace Canal

#endif // LIBCANAL_CONSTRUCTORS_H
