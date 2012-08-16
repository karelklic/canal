#ifndef LIBCANAL_INTEGER_RANGE_H
#define LIBCANAL_INTEGER_RANGE_H

#include "Value.h"
#include <llvm/Constants.h>

namespace Canal {
namespace Integer {

// Abstracts integer values as a range min - max.
class Range : public Value, public AccuracyValue
{
public:
    bool mEmpty;

    bool mSignedTop;
    // The number is included in the interval.
    llvm::APInt mSignedFrom;
    // The number is included in the interval.
    llvm::APInt mSignedTo;

    bool mUnsignedTop;
    // The number is included in the interval.
    llvm::APInt mUnsignedFrom;
    // The number is included in the interval.
    llvm::APInt mUnsignedTo;

public:
    // Initializes to the lowest value.
    Range(unsigned numBits);
    Range(const llvm::APInt &constant);

    unsigned getBitWidth() const { return mSignedFrom.getBitWidth(); }

    // Lowest signed number represented by this abstract domain.
    // @param result
    //   Filled by the minimum value if it is known.  Otherwise, the
    //   value is undefined.
    // @return
    //   True if the result is known and the parameter was set to
    //   correct value.
    bool signedMin(llvm::APInt &result) const;

    // Highest signed number represented by this abstract domain.
    // @param result
    //   Filled by the maximum value if it is known.  Otherwise, the
    //   value is undefined.
    // @return
    //   True if the result is known and the parameter was set to
    //   correct value.
    bool signedMax(llvm::APInt &result) const;

    // Lowest unsigned number represented by this abstract domain.
    // @param result
    //   Filled by the minimum value if it is known.  Otherwise, the
    //   value is undefined.
    // @return
    //   True if the result is known and the parameter was set to
    //   correct value.
    bool unsignedMin(llvm::APInt &result) const;

    // Highest unsigned number represented by this abstract domain.
    // @param result
    //   Filled by the maximum value if it is known.  Otherwise, the
    //   value is undefined.
    // @return
    //   True if the result is known and the parameter was set to
    //   correct value.
    bool unsignedMax(llvm::APInt &result) const;

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Range *clone() const;
    // Implementation of Value::cloneCleaned().
    // Covariant return type.
    virtual Range *cloneCleaned() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString() const;

    // Implementation of Value::add().
    virtual void add(const Value &a, const Value &b);
    // Implementation of Value::sub().
    virtual void sub(const Value &a, const Value &b);
    // Implementation of Value::mul().
    virtual void mul(const Value &a, const Value &b);
    // Implementation of Value::udiv().
    virtual void udiv(const Value &a, const Value &b);
    // Implementation of Value::sdiv().
    virtual void sdiv(const Value &a, const Value &b);
    // Implementation of Value::urem().
    virtual void urem(const Value &a, const Value &b);
    // Implementation of Value::srem().
    virtual void srem(const Value &a, const Value &b);
    // Implementation of Value::shl().
    virtual void shl(const Value &a, const Value &b);
    // Implementation of Value::lshr().
    virtual void lshr(const Value &a, const Value &b);
    // Implementation of Value::ashr().
    virtual void ashr(const Value &a, const Value &b);
    // Implementation of Value::and_().
    virtual void and_(const Value &a, const Value &b);
    // Implementation of Value::or_().
    virtual void or_(const Value &a, const Value &b);
    // Implementation of Value::xor_().
    virtual void xor_(const Value &a, const Value &b);
    // Implementation of Value::icmp().
    virtual void icmp(const Value &a, const Value &b,
                      llvm::CmpInst::Predicate predicate);

public: // Implementation of AccuracyValue.
    // Implementation of AccuracyValue::accuracy().
    virtual float accuracy() const;
    // Implementation of AccuracyValue::isBottom().
    virtual bool isBottom() const;
    // Implementation of AccuracyValue::setBottom().
    virtual void setBottom();
    // Implementation of AccuracyValue::isTop().
    virtual bool isTop() const;
    // Implementation of AccuracyValue::setTop().
    virtual void setTop();

protected:
    bool intersection(const Range& a, const bool s = true, const bool u = true) const;

    bool isSingleValue() const;
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_RANGE_H
