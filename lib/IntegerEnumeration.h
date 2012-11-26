#ifndef LIBCANAL_INTEGER_ENUMERATION_H
#define LIBCANAL_INTEGER_ENUMERATION_H

#include "Domain.h"
#include "APIntUtils.h"

namespace Canal {
namespace Integer {

class Enumeration : public Domain
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
                const llvm::APInt &constant);

    /// Copy constructor.
    Enumeration(const Enumeration &value);

    unsigned getBitWidth() const
    {
        return mBitWidth;
    }

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

    /// Does the enumeration represent signle bit that is set to 1?
    bool isTrue() const;

    /// Does the enumeration represent signle bit that is set to 0?
    bool isFalse() const;

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Enumeration *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain& value) const;

    virtual bool operator<(const Domain &value) const;

    virtual bool operator>(const Domain &value) const;

    virtual Enumeration &join(const Domain &value);

    virtual Enumeration &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual Enumeration &add(const Domain &a, const Domain &b);

    virtual Enumeration &sub(const Domain &a, const Domain &b);

    virtual Enumeration &mul(const Domain &a, const Domain &b);

    virtual Enumeration &udiv(const Domain &a, const Domain &b);

    virtual Enumeration &sdiv(const Domain &a, const Domain &b);

    virtual Enumeration &urem(const Domain &a, const Domain &b);

    virtual Enumeration &srem(const Domain &a, const Domain &b);

    virtual Enumeration &shl(const Domain &a, const Domain &b);

    virtual Enumeration &lshr(const Domain &a, const Domain &b);

    virtual Enumeration &ashr(const Domain &a, const Domain &b);

    virtual Enumeration &and_(const Domain &a, const Domain &b);

    virtual Enumeration &or_(const Domain &a, const Domain &b);

    virtual Enumeration &xor_(const Domain &a, const Domain &b);

    virtual Enumeration &icmp(const Domain &a, const Domain &b,
                              llvm::CmpInst::Predicate predicate);

    virtual Enumeration &fcmp(const Domain &a, const Domain &b,
                              llvm::CmpInst::Predicate predicate);

    virtual Enumeration &trunc(const Domain &value);

    virtual Enumeration &zext(const Domain &value);

    virtual Enumeration &sext(const Domain &value);

    virtual Enumeration &fptoui(const Domain &value);

    virtual Enumeration &fptosi(const Domain &value);

protected:
    Enumeration &applyOperation(const Domain &a,
                                const Domain &b,
                                APIntUtils::Operation operation1,
                                APIntUtils::OperationWithOverflow operation2);
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_ENUMERATION_H
