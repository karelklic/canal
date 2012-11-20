#ifndef LIBCANAL_ARRAY_STRING_PREFIX_H
#define LIBCANAL_ARRAY_STRING_PREFIX_H

#include "Domain.h"
#include "ArrayInterface.h"

namespace Canal {
namespace Array {

/// Array with exact size and limited length.  It keeps all array
/// members separately, not losing precision at all.
class StringPrefix : public Domain, public Interface
{
public:
    std::string mPrefix;
    bool mIsBottom;

public:
    /// Standard constructor
    StringPrefix(const Environment &environment);

    /// @param value
    ///   This class does not take ownership of this value.
    StringPrefix(const Environment &environment,
              const std::string &value);

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    StringPrefix &operator=(const StringPrefix &value);

public: // Implementation of Domain.
    /// Implementation of Domain::clone().
    /// Covariant return type.
    virtual StringPrefix *clone() const;
    /// Implementation of Domain::cloneCleaned().
    /// Covariant return type.
    virtual StringPrefix *cloneCleaned() const;
    /// Implementation of Domain::operator==().
    virtual bool operator==(const Domain &value) const;
    /// Implementation of Domain::merge().
    virtual void merge(const Domain &value);
    /// Implementation of Domain::memoryUsage().
    virtual size_t memoryUsage() const;
    /// Implementation of Domain::toString().
    virtual std::string toString() const;

    /// Implementation of Domain::setZero()
    virtual void setZero(const llvm::Value *place);

    /// Implementation of Domain::add().
    virtual void add(const Domain &a, const Domain &b);
    /// Implementation of Domain::fadd().
    virtual void fadd(const Domain &a, const Domain &b);
    /// Implementation of Domain::sub().
    virtual void sub(const Domain &a, const Domain &b);
    /// Implementation of Domain::fsub().
    virtual void fsub(const Domain &a, const Domain &b);
    /// Implementation of Domain::mul().
    virtual void mul(const Domain &a, const Domain &b);
    /// Implementation of Domain::fmul().
    virtual void fmul(const Domain &a, const Domain &b);
    /// Implementation of Domain::udiv().
    virtual void udiv(const Domain &a, const Domain &b);
    /// Implementation of Domain::sdiv().
    virtual void sdiv(const Domain &a, const Domain &b);
    /// Implementation of Domain::fdiv().
    virtual void fdiv(const Domain &a, const Domain &b);
    /// Implementation of Domain::urem().
    virtual void urem(const Domain &a, const Domain &b);
    /// Implementation of Domain::srem().
    virtual void srem(const Domain &a, const Domain &b);
    /// Implementation of Domain::frem().
    virtual void frem(const Domain &a, const Domain &b);
    /// Implementation of Domain::shl().
    virtual void shl(const Domain &a, const Domain &b);
    /// Implementation of Domain::lshr().
    virtual void lshr(const Domain &a, const Domain &b);
    /// Implementation of Domain::ashr().
    virtual void ashr(const Domain &a, const Domain &b);
    /// Implementation of Domain::and_().
    virtual void and_(const Domain &a, const Domain &b);
    /// Implementation of Domain::or_().
    virtual void or_(const Domain &a, const Domain &b);
    /// Implementation of Domain::xor_().
    virtual void xor_(const Domain &a, const Domain &b);
    /// Implementation of Domain::icmp().
    virtual void icmp(const Domain &a, const Domain &b,
                      llvm::CmpInst::Predicate predicate);
    /// Implementation of Domain::fcmp().
    virtual void fcmp(const Domain &a, const Domain &b,
                      llvm::CmpInst::Predicate predicate);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

public: // Implementation of Array::Interface.
    /// Implementation of Array::Interface::getItem().
    virtual std::vector<Domain*> getItem(const Domain &offset) const;
    /// Implementation of Array::Interface::getItem().
    virtual Domain *getItem(uint64_t offset) const;
    /// Implementation of Array::Interface::setItem().
    virtual void setItem(const Domain &offset, const Domain &value);
    /// Implementation of Array::Interface::setItem().
    virtual void setItem(uint64_t offset, const Domain &value);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_STRING_PREFIX_H