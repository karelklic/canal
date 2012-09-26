#ifndef LIBCANAL_INTEGER_ENUMERATION_H
#define LIBCANAL_INTEGER_ENUMERATION_H

#include "Domain.h"
#include "APIntUtils.h"

namespace Canal {
namespace Integer {

class Enumeration : public Domain, public AccuracyDomain
{
public:
    APIntUtils::USet mValues;
    bool mTop;
    unsigned mBitWidth;
    static const unsigned int mMaxSize = 40;

public:
    /// Initializes to the lowest value.
    Enumeration(const Environment &environment,
                unsigned bitWidth);

    /// Initializes to the given value.
    Enumeration(const Environment &environment,
                const llvm::APInt &number);

    unsigned getBitWidth() const { return mBitWidth; }

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

    /// Does this enumeration represent single value?
    bool isSingleValue() const;

public: // Implementation of Domain.
    /// Implementation of Domain::clone().
    /// Covariant return type.
    virtual Enumeration *clone() const;
    /// Implementation of Domain::cloneCleaned().
    /// Covariant return type.
    virtual Enumeration *cloneCleaned() const;
    /// Implementation of Domain::operator==().
    virtual bool operator==(const Domain& value) const;
    /// Implementation of Domain::merge().
    virtual void merge(const Domain &value);
    /// Implementation of Domain::memoryUsage().
    virtual size_t memoryUsage() const;
    /// Implementation of Domain::toString().
    virtual std::string toString() const;

    /// Implementation of Domain::matchesString().
    /// Examples of allowed input:
    ///   enumeration 3 -4 5
    ///   enumeration 0xff 0xfa
    ///   enumeration
    ///     0x1000000F
    ///     0xABABABAB
    ///   enumeration top
    ///   enumeration empty
    ///   enumeration
    ///     empty
    virtual bool matchesString(const std::string &text,
                               std::string &rationale) const;

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

public: // Implementation of AccuracyDomain.
    /// Implementation of AccuracyDomain::accuracy().
    virtual float accuracy() const;
    /// Implementation of AccuracyDomain::isBottom().
    virtual bool isBottom() const;
    /// Implementation of AccuracyDomain::setBottom().
    virtual void setBottom();
    /// Implementation of AccuracyDomain::isTop().
    virtual bool isTop() const;
    /// Implementation of AccuracyDomain::setTop().
    virtual void setTop();

protected:
    void applyOperation(const Domain &a,
                        const Domain &b,
                        APIntUtils::Operation operation1,
                        APIntUtils::OperationWithOverflow operation2);
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_ENUMERATION_H
