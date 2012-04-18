#ifndef CANAL_INTEGER_BITS_H
#define CANAL_INTEGER_BITS_H

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
class Bits : public Value
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

    unsigned getBitWidth() const { return mBits0.getBitWidth(); }

    // Implementation of Value::clone().
    // Covariant return type.
    virtual Bits *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;

    virtual float accuracy() const;
    virtual bool isBottom() const;
    virtual void setTop();

    void and_(const Value &a, const Value &b);
    void or_(const Value &a, const Value &b);
    void xor_(const Value &a, const Value &b);

    bool hasBit(unsigned pos) const;

    // If this bit is set (hasBit(pos) == true), it will return its
    // value.  Otherwise it will return 0 if it can represent both
    // values, 1 if unset.
    bool getBit(unsigned pos) const;

    void setBit(unsigned pos, bool bit);

    // Number of definitely known bits.
    unsigned bitcount() const;

protected:
    void bitOp(const Bits &a, const Bits &b, void(*fun)(bool,bool,bool&,bool&));

};

} // namespace Integer
} // namespace Canal

#endif // CANAL_INTEGER_BITS_H
