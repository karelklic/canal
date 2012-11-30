#ifndef LIBCANAL_INTEGER_SET_H
#define LIBCANAL_INTEGER_SET_H

#include "Domain.h"
#include "APIntUtils.h"

namespace Canal {
namespace Integer {

class Set : public Domain
{
public:
    APIntUtils::USet mValues;

    bool mTop;

    unsigned mBitWidth;

    static unsigned int SET_THRESHOLD;

public:
    /// Initializes to the lowest value.
    Set(const Environment &environment,
                unsigned bitWidth);

    /// Initializes to the given value.
    Set(const Environment &environment,
                const llvm::APInt &constant);

    /// Copy constructor.
    Set(const Set &value);

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

    /// Does this set represent single value?
    bool isConstant() const;

    /// Does the set represent signle bit that is set to 1?
    bool isTrue() const;

    /// Does the set represent signle bit that is set to 0?
    bool isFalse() const;

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Set *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain& value) const;

    virtual bool operator<(const Domain &value) const;

    virtual bool operator>(const Domain &value) const;

    virtual Set &join(const Domain &value);

    virtual Set &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual Set &add(const Domain &a, const Domain &b);

    virtual Set &sub(const Domain &a, const Domain &b);

    virtual Set &mul(const Domain &a, const Domain &b);

    virtual Set &udiv(const Domain &a, const Domain &b);

    virtual Set &sdiv(const Domain &a, const Domain &b);

    virtual Set &urem(const Domain &a, const Domain &b);

    virtual Set &srem(const Domain &a, const Domain &b);

    virtual Set &shl(const Domain &a, const Domain &b);

    virtual Set &lshr(const Domain &a, const Domain &b);

    virtual Set &ashr(const Domain &a, const Domain &b);

    virtual Set &and_(const Domain &a, const Domain &b);

    virtual Set &or_(const Domain &a, const Domain &b);

    virtual Set &xor_(const Domain &a, const Domain &b);

    virtual Set &icmp(const Domain &a, const Domain &b,
                              llvm::CmpInst::Predicate predicate);

    virtual Set &fcmp(const Domain &a, const Domain &b,
                              llvm::CmpInst::Predicate predicate);

    virtual Set &trunc(const Domain &value);

    virtual Set &zext(const Domain &value);

    virtual Set &sext(const Domain &value);

    virtual Set &fptoui(const Domain &value);

    virtual Set &fptosi(const Domain &value);

    virtual bool isValue() const
    {
        return true;
    }

    /// Covariant return type.
    virtual const llvm::IntegerType &getValueType() const;

    virtual bool hasValueExactSize() const
    {
        return true;
    }

    virtual Domain *getValueCell(uint64_t offset) const;

    virtual void mergeValueCell(uint64_t offset, const Domain &value);

protected:
    Set &applyOperation(const Domain &a,
                                const Domain &b,
                                APIntUtils::Operation operation1,
                                APIntUtils::OperationWithOverflow operation2);
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_SET_H
