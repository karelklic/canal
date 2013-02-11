#ifndef LIBCANAL_CONSTRUCTORS_H
#define LIBCANAL_CONSTRUCTORS_H

#include "Prereq.h"
#include <inttypes.h>
#include <vector>

namespace Canal {

class Constructors
{
    const Environment &mEnvironment;

public:
    Constructors(Environment &environment);

    const Environment &getEnvironment() const
    {
        return mEnvironment;
    }

    Domain *create(const llvm::Type &type) const;

    /// @brief
    ///   Create an abstract value from a given constant.
    ///
    /// Converts a LLVM constant to an abstract value of the
    /// corresponding type.
    ///
    /// @param state
    ///   State is used only for constant expressions such as
    ///   getelementptr and bitcast.  For other types of constants it
    ///   might be NULL.  Constant expressions require state because
    ///   they might refer to other abstract variables that are stored
    ///   in the state.
    /// @returns
    ///   Returns a newly allocated value.  Caller takes ownership of
    ///   the returned value.  It never returns NULL.
    Domain *create(const llvm::Constant &value,
                   const Memory::State *state) const;

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
                               const Memory::State *state) const;

    Domain *createGetElementPtr(const llvm::ConstantExpr &value,
                                const std::vector<const Domain*> &operands) const;

    Domain *createBitCast(const llvm::ConstantExpr &value,
                          const std::vector<const Domain*> &operands) const;
};

} // namespace Canal

#endif // LIBCANAL_CONSTRUCTORS_H
