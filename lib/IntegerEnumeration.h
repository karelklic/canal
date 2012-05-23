#ifndef LIBCANAL_INTEGER_ENUMERATION_H
#define LIBCANAL_INTEGER_ENUMERATION_H

#include "Value.h"
#include <llvm/ADT/APInt.h>
#include <set>

namespace Canal {
namespace Integer {

struct APIntComp
{
  bool operator()(const llvm::APInt &a, const llvm::APInt &b) const
  {
      return a.ult(b);
  }
};

typedef std::set<llvm::APInt, APIntComp> APIntSet;

class Enumeration : public AccuracyValue
{
public:
    APIntSet mValues;
    bool mTop;

public:
    // Initializes to the lowest value.
    Enumeration();
    // Initializes to the given value.
    Enumeration(const llvm::APInt &number);

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Enumeration *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;

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
    typedef llvm::APInt(llvm::APInt::*APIntOperation)(const llvm::APInt&) const;
    typedef llvm::APInt(llvm::APInt::*APIntOperationWithOverflow)(const llvm::APInt&, bool&) const;

    void applyOperation(const Value &a,
                        const Value &b,
                        APIntOperation operation1,
                        APIntOperationWithOverflow operation2);
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_ENUMERATION_H
