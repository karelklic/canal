#ifndef LIBCANAL_INTEGER_BITS_H
#define LIBCANAL_INTEGER_BITS_H

#include "Value.h"
#include <llvm/ADT/APInt.h>

namespace Canal {
namespace Integer {

// Abstracts integers as a bitfield.
//
// For every bit, we have 4 possible states:
//  mBits0  mBits1  State
// -----------------------
//    0        0    Nothing was set to the bit (lowest lattice value - bottom)
//    1        0    The bit is set to 0
//    0        1    The bit is set to 1
//    1        1    The bit can be both 0 and 1 (highest lattice value - top)
class Bits : public Value, public AccuracyValue
{
public:
    // When a bit in mBits0 is 1, the value is known to contain zero
    // at this position.
    llvm::APInt mBits0;
    // When a bit in mBits1 is 1, the value is known to contain one at
    // this position.
    llvm::APInt mBits1;

public:
    // Initializes to the lowest value.
    Bits(unsigned numBits);
    // Initializes to the given value.
    Bits(const llvm::APInt &number);

    // Return the number of bits of the represented number.
    unsigned getBitWidth() const { return mBits0.getBitWidth(); }

    // Returns 0 if the bit is known to be 0.  Returns 1 if the bit is
    // known to be 1.  Returns -1 if the bit value is unknown.
    // Returns 2 if the bit is either 1 or 0.
    int getBitValue(unsigned pos) const;

    // Sets the bit.  If value is 0 or 1, the bit is set to represent
    // exactly 0 or 1.  If value is -1, the bit is set to represent
    // unknown value.  If value is 2, the bit is set to represent both
    // 0 and 1.
    void setBitValue(unsigned pos, int value);

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Bits *clone() const;
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
    // Implementation of AccuracyValue::setTop().
    virtual void setTop();
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_BITS_H
