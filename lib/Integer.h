#ifndef LIBCANAL_INTEGER_H
#define LIBCANAL_INTEGER_H

#include "Value.h"

namespace llvm {
    class APInt;
}

namespace Canal {
namespace Integer {

class Bits;
class Enumeration;
class Range;

class Container : public Value, public AccuracyValue
{
public:
    std::vector<Value*> mValues;

public:
    Container(unsigned numBits);
    // Creates a new container with an initial value.  Signedness,
    // number of bits is taken from the provided number.
    Container(const llvm::APInt &number);
    // Copy constructor.  Creates independent copy of the container.
    Container(const Container &container);
    // Destructor.  Deletes the contents of the container.
    virtual ~Container();

    Bits &getBits();
    const Bits &getBits() const;

    Enumeration &getEnumeration();
    const Enumeration &getEnumeration() const;

    Range &getRange();
    const Range &getRange() const;

    // Lowest signed number represented by this container.  Uses the
    // abstract domain (enum, range, bits) with highest precision.
    llvm::APInt signedMin() const;
    // Highest signed number represented by this container.  Uses the
    // abstract domain (enum, range, bits) with highest precision.
    llvm::APInt signedMax() const;
    // Lowest unsigned number represented by this container.  Uses the
    // abstract domain (enum, range, bits) with highest precision.
    llvm::APInt unsignedMin() const;
    // Highest unsigned number represented by this container.  Uses
    // the abstract domain (enum, range, bits) with highest precision.
    llvm::APInt unsignedMax() const;

public: // Implementation of Value.
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
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_H
