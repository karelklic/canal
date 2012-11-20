#ifndef LIBCANAL_INTEGER_INTERVAL_H
#define LIBCANAL_INTEGER_INTERVAL_H

#include "Domain.h"

namespace Canal {
namespace Integer {

/// Abstracts integer values as a interval min - max.
class Interval : public Domain
{
public:
    /// @brief Indicates an empty interval.
    ///
    /// When it is set to true, other members' values are not
    /// considered as valid.
    bool mEmpty;

    bool mSignedTop;
    /// The number is included in the interval.
    llvm::APInt mSignedFrom;
    /// The number is included in the interval.
    llvm::APInt mSignedTo;

    bool mUnsignedTop;
    /// The number is included in the interval.
    llvm::APInt mUnsignedFrom;
    /// The number is included in the interval.
    llvm::APInt mUnsignedTo;

public:
    /// @brief Standard constructor.
    ///
    /// Initializes an empty interval.
    Interval(const Environment &environment,
             unsigned bitWidth);

    /// @brief Standard constructor.
    Interval(const Environment &environment,
             const llvm::APInt &constant);

    /// Copy constructor.
    Interval(const Interval &value);

    unsigned getBitWidth() const { return mSignedFrom.getBitWidth(); }

    /// Lowest signed number represented by this abstract domain.
    /// @param result
    ///   Filled by the minimum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool signedMin(llvm::APInt &result) const;

    /// Highest signed number represented by this abstract domain.
    /// @param result
    ///   Filled by the maximum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool signedMax(llvm::APInt &result) const;

    /// Lowest unsigned number represented by this abstract domain.
    /// @param result
    ///   Filled by the minimum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool unsignedMin(llvm::APInt &result) const;

    /// Highest unsigned number represented by this abstract domain.
    /// @param result
    ///   Filled by the maximum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool unsignedMax(llvm::APInt &result) const;

    /// Returns true if the interval represents a single number.  Signed
    /// and unsigned representations might differ, though.
    bool isSingleValue() const;

    /// Returns true if the interval represents a signed single value.
    bool isSignedSingleValue() const;

    /// Returns true if the interval represents a unsigned single value.
    bool isUnsignedSingleValue() const;

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Interval &operator=(const Interval &value);

public: // Implementation of Domain.
    /// Implementation of Domain::clone().
    /// Covariant return type.
    virtual Interval *clone() const;
    /// Implementation of Domain::operator==().
    virtual bool operator==(const Domain& value) const;
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

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_INTERVAL_H
