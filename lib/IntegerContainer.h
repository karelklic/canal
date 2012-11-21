#ifndef LIBCANAL_INTEGER_CONTAINER_H
#define LIBCANAL_INTEGER_CONTAINER_H

#include "Domain.h"
#include "IntegerBitfield.h"
#include "IntegerEnumeration.h"
#include "IntegerInterval.h"

namespace Canal {
namespace Integer {

class Container : public Domain
{
public:
    std::vector<Domain*> mValues;

public:
    Container(const Environment &environment,
              unsigned bitWidth);

    /// Creates a new container with an initial value.  Signedness,
    /// number of bits is taken from the provided number.
    Container(const Environment &environment,
              const llvm::APInt &number);

    /// Copy constructor.  Creates independent copy of the container.
    Container(const Container &value);

    /// Destructor.  Deletes the contents of the container.
    virtual ~Container();

    unsigned getBitWidth() const;

    Canal::Integer::Bitfield_type &getBitfield();
    const Bitfield_type &getBitfield() const;

    Enumeration &getEnumeration();
    const Enumeration &getEnumeration() const;

    Interval &getInterval();
    const Interval &getInterval() const;

    /// Lowest signed number represented by this container.  Uses the
    /// abstract domain (enum, interval, bits) with highest precision.
    /// @param result
    ///   Filled by the minimum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool signedMin(llvm::APInt &result) const;

    /// Highest signed number represented by this container.  Uses the
    /// abstract domain (enum, interval, bits) with highest precision.
    /// @param result
    ///   Filled by the maximum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool signedMax(llvm::APInt &result) const;

    /// Lowest unsigned number represented by this container.  Uses the
    /// abstract domain (enum, interval, bits) with highest precision.
    /// @param result
    ///   Filled by the minimum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool unsignedMin(llvm::APInt &result) const;

    /// Highest unsigned number represented by this container.  Uses
    /// the abstract domain (enum, interval, bits) with highest precision.
    /// @param result
    ///   Filled by the maximum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool unsignedMax(llvm::APInt &result) const;

    /// Find out whether all representations contain only single value
    bool isSingleValue() const;

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Container &operator=(const Container &value);

public: // Implementation of Domain.
    /// Implementation of Domain::clone().
    /// Covariant return type.
    virtual Container *clone() const;
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

    /// Implementation of Domain::add().
    virtual void add(const Domain &a, const Domain &b);
    /// Implementation of Domain::sub().
    virtual void sub(const Domain &a, const Domain &b);
    /// Implementation of Domain::mul().
    virtual void mul(const Domain &a, const Domain &b);
    /// Implementation of Domain::udiv().
    virtual void udiv(const Domain &a, const Domain &b);
    /// Implementation of Domain::sdiv().
    virtual void sdiv(const Domain &a, const Domain &b);
    /// Implementation of Domain::urem().
    virtual void urem(const Domain &a, const Domain &b);
    /// Implementation of Domain::srem().
    virtual void srem(const Domain &a, const Domain &b);
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
    virtual void trunc(const Domain &value);
    virtual void zext(const Domain &value);
    virtual void sext(const Domain &value);
    virtual void fptoui(const Domain &value);
    virtual void fptosi(const Domain &value);

    virtual float accuracy() const;

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();
};
DONT_USE_COW(Container);
} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_H
