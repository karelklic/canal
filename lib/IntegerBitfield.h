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
    bool isConstant() const;

    /// Does the interval represent signle bit that is set to 1?
    bool isTrue() const;

    /// Does the interval represent signle bit that is set to 0?
    bool isFalse() const;

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Bitfield *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain& value) const;

    virtual bool operator<(const Domain &value) const;

    virtual bool operator>(const Domain &value) const;

    virtual Bitfield &join(const Domain &value);

    virtual Bitfield &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual Bitfield &add(const Domain &a, const Domain &b);

    virtual Bitfield &sub(const Domain &a, const Domain &b);

    virtual Bitfield &mul(const Domain &a, const Domain &b);

    virtual Bitfield &udiv(const Domain &a, const Domain &b);

    virtual Bitfield &sdiv(const Domain &a, const Domain &b);

    virtual Bitfield &urem(const Domain &a, const Domain &b);

    virtual Bitfield &srem(const Domain &a, const Domain &b);

    virtual Bitfield &shl(const Domain &a, const Domain &b);

    virtual Bitfield &lshr(const Domain &a, const Domain &b);

    virtual Bitfield &ashr(const Domain &a, const Domain &b);

    virtual Bitfield &and_(const Domain &a, const Domain &b);

    virtual Bitfield &or_(const Domain &a, const Domain &b);

    virtual Bitfield &xor_(const Domain &a, const Domain &b);

    virtual Bitfield &icmp(const Domain &a, const Domain &b,
                           llvm::CmpInst::Predicate predicate);

    virtual Bitfield &fcmp(const Domain &a, const Domain &b,
                           llvm::CmpInst::Predicate predicate);

    virtual Bitfield &trunc(const Domain &value);

    virtual Bitfield &zext(const Domain &value);

    virtual Bitfield &sext(const Domain &value);

    virtual Bitfield &fptoui(const Domain &value);

    virtual Bitfield &fptosi(const Domain &value);
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_BITFIELD_H
