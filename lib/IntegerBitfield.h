#ifndef LIBCANAL_INTEGER_BITFIELD_H
#define LIBCANAL_INTEGER_BITFIELD_H

#include "Domain.h"

namespace Canal {
namespace Integer {

/// Abstracts integers as a bitfield.
///
/// For every bit, we have 4 possible states:
///  mZeroes  mOnes  State
/// -----------------------
///    0        0    Nothing was set to the bit (lowest lattice value - bottom)
///    1        0    The bit is set to 0
///    0        1    The bit is set to 1
///    1        1    The bit can be both 0 and 1 (highest lattice value - top)
class Bitfield : public Domain
{
public:
    /// When a bit in mZeroes is 1, the value is known to contain zero
    /// at this position.
    llvm::APInt mZeroes;
    /// When a bit in mOnes is 1, the value is known to contain one at
    /// this position.
    llvm::APInt mOnes;

public:
    /// Initializes to the lowest value.
    Bitfield(const Environment &environment,
             unsigned bitWidth);

    /// Initializes to the given value.
    Bitfield(const Environment &environment,
             const llvm::APInt &number);

    /// Copy constructor.
    Bitfield(const Bitfield &value);

    /// Return the number of bits of the represented number.
    unsigned getBitWidth() const
    {
        return mZeroes.getBitWidth();
    }

    /// Returns 0 if the bit is known to be 0.  Returns 1 if the bit is
    /// known to be 1.  Returns -1 if the bit value is unknown.
    /// Returns 2 if the bit is either 1 or 0.
    int getBitValue(unsigned pos) const;

    /// Sets the bit.  If value is 0 or 1, the bit is set to represent
    /// exactly 0 or 1.  If value is -1, the bit is set to represent
    /// unknown value.  If value is 2, the bit is set to represent both
    /// 0 and 1.
    void setBitValue(unsigned pos, int value);

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

    /// Does these bits represent single value?
    bool isSingleValue() const;

public: // Implementation of Domain.
    /// Implementation of Domain::clone().
    /// Covariant return type.
    virtual Bitfield *clone() const;
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

#endif // LIBCANAL_INTEGER_BITFIELD_H
