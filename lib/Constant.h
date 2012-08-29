#ifndef LIBCANAL_CONSTANT_H
#define LIBCANAL_CONSTANT_H

#include "Domain.h"

namespace llvm {
    class Constant;
    class APInt;
}

namespace Canal {

class Domain;

class Constant : public Domain
{
public:
    const llvm::Constant *mConstant;

public:
    Constant(const Environment &environment, const llvm::Constant *constant);

    // Checks if this constant is an integer number.
    bool isAPInt() const;
    // Returns number for integer constant.
    const llvm::APInt &getAPInt() const;

    bool isNullPtr() const;

    bool isGetElementPtr() const;

    // Creates a modifiable value from the constant.  Caller is
    // responsible for releasing its memory.
    Domain *toModifiableValue() const;

public: // Implementation of Domain.
    // Implementation of Domain::clone().
    // Covariant return type.
    virtual Constant *clone() const;
    // Implementation of Domain::cloneCleaned().
    // Covariant return type.
    virtual Constant *cloneCleaned() const;
    // Implementation of Domain::operator==().
    virtual bool operator==(const Domain &value) const;
    // Implementation of Domain::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Domain::toString().
    virtual std::string toString() const;
    // Implementation of Domain::matchesString().
    virtual bool matchesString(const std::string &text,
                               std::string &rationale) const;
    virtual void merge(const Domain &value);
};

} // namespace Canal

#endif // LIBCANAL_CONSTANT_H
