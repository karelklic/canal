#ifndef LIBCANAL_INTEGER_H
#define LIBCANAL_INTEGER_H

#include "IntegerBits.h"
//#include "IntegerEnumeration.h"
//#include "IntegerRange.h"

namespace llvm {
    class APInt;
}

namespace Canal {
namespace Integer {

class Container : public AccuracyValue
{
public:
    Container(unsigned numBits);
    // Creates a new container with an initial value.  Signedness,
    // number of bits is taken from the provided number.
    Container(const llvm::APInt &number);
    // Copy constructor.  Creates independent copy of the container.
    Container(const Container &container);
    virtual ~Container();

    // Implementation of Value::clone().
    // Covariant return type.
    virtual Container *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
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

public: // Integer
    Bits &getBits();
    const Bits &getBits() const;

public:
    std::vector<AccuracyValue*> mValues;
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_H
