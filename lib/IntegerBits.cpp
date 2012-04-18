#include "IntegerBits.h"
#include "Constant.h"
#include "Utils.h"
#include <llvm/Support/raw_ostream.h>

namespace Canal {
namespace Integer {

Bits::Bits(unsigned numBits) : mBits0(numBits, 0), mBits1(numBits, 0)
{
}

Bits::Bits(const llvm::APInt &number) : mBits0(~number), mBits1(number)
{
}

bool Bits::operator==(const Value& value) const
{
    const Bits *other = dynamic_cast<const Bits*>(&value);
    if (!other)
        return false;
    return mBits0 == other->mBits0 && mBits1 == other->mBits1;
}

Bits *Bits::clone() const
{
    return new Bits(*this);
}

void Bits::merge(const Value &value)
{
    const Constant *constant = dynamic_cast<const Constant*>(&value);
    if (constant)
    {
        CANAL_ASSERT(constant->isAPInt());
        mBits1 |= constant->getAPInt();
    }

    const Bits &bits = dynamic_cast<const Bits&>(value);
    mBits0 |= bits.mBits0;
    mBits1 |= bits.mBits1;
}

float Bits::accuracy() const
{
    return 1.0 - (this->bitcount() / (float)mBits0.getBitWidth());
}

bool Bits::isBottom() const
{
    return mBits0 == 0 && mBits1 == 0;
}

void Bits::setTop()
{
    mBits0 = mBits1 = ~0;
}

void Bits::printToStream(llvm::raw_ostream &ostream) const
{
    ostream << "Integer::Bits(";
    for (int pos = 0; pos < mBits0.getBitWidth(); ++pos)
    {
        if (hasBit(pos))
            ostream << getBit(pos);
        else
            ostream << (this->getBit(pos) ? '_' : '?');
    }
    ostream << ")";
}

static void bitAnd(bool know1, bool know2, bool &bit0, bool &bit1)
{
    switch ((unsigned)know1 + (unsigned)know2)
    {
    case 2:
        if (bit0 && bit1)
        {
            bit0 = 1;
            bit1 = 0;
        }
        else
        {
            bit0 = 0;
            bit1 = 1;
        }
        break;
    case 1:
          if ((know1 && !bit0) || (know2 && !bit1))
          {
              // One of the bits is 0, so the result is 0
              bit0 = 1; bit1 = 0;
          }
          else
              bit0 = bit1 = 1;
          break;
    default:
        // No info
        bit0 = bit1 = 1;
        break;
    }
}

void Bits::and_(const Value &a, const Value &b)
{
    const Bits &aa = dynamic_cast<const Bits&>(a),
        &bb = dynamic_cast<const Bits&>(b);

    bitOp(aa, bb, bitAnd);
}

static void bitOr(bool know1, bool know2, bool &bit0, bool &bit1)
{
    switch ((unsigned)know1 + (unsigned)know2)
    {
    case 2:
        if (bit0 || bit1)
        {
            bit0 = 0;
            bit1 = 1;
        }
        else
        {
            bit0 = 1;
            bit1 = 0;
        }
        break;
    case 1:
        if ((know1 && bit0) || (know2 && bit1))
        {
            // One of the bits is 1, so the result is 1
            bit0 = 0;
            bit1 = 1;
        }
        else
            bit0 = bit1 = 1;
        break;
    default:
        // No info
        bit0 = bit1 = 1;
    }
}

void Bits::or_(const Value &a, const Value &b)
{
    const Bits &aa = dynamic_cast<const Bits&>(a),
        &bb = dynamic_cast<const Bits&>(b);

    bitOp(aa, bb, bitOr);
}


// First number in a pair is mBits1, second is mBits0
// 00 xor 00 = 00
// 00 xor 01 = 00
// 00 xor 10 = 10
// 00 xor 11 = 11
// 10 xor 01 = 10
// 10 xor 10 = 01
// 10 xor 11 = 11
// 01 xor 01 = 01
// 01 xor 11 = 11
// 11 xor 11 = 11
static void bitXor(bool know1, bool know2, bool &bit0, bool &bit1)
{
    switch ((unsigned)know1 + (unsigned)know2)
    {
    case 2:
        if (bit0 xor bit1)
        {
            bit0 = 0;
            bit1 = 1;
        }
        else
        {
            bit0 = 1;
            bit1 = 0;
        }
        break;
    default:
        // No info
        bit0 = bit1 = 1;
    }
}

void Bits::xor_(const Value &a, const Value &b)
{
    const Bits &aa = dynamic_cast<const Bits&>(a),
        &bb = dynamic_cast<const Bits&>(b);
    bitOp(aa, bb, bitXor);
}

bool Bits::hasBit(unsigned pos) const
{
    llvm::APInt bit(llvm::APInt::getOneBitSet(mBits0.getBitWidth(), pos));
    return ((mBits0 & bit) xor (mBits1 & bit)).getBoolValue();
}

bool Bits::getBit(unsigned pos) const
{
    llvm::APInt bit(llvm::APInt::getOneBitSet(mBits0.getBitWidth(), pos));
    return !(mBits0 & bit).getBoolValue();
}

void Bits::setBit(unsigned pos, bool bit)
{
    llvm::APInt bitpos(llvm::APInt::getOneBitSet(mBits0.getBitWidth(), pos));
    llvm::APInt &where = (bit ? mBits1 : mBits0);
    llvm::APInt &other = (bit ? mBits0 : mBits1);
    where |= bitpos;
    other &= ~bitpos;
}

unsigned Bits::bitcount() const
{
    unsigned ret = 0;
    for (unsigned i = 0; i < mBits0.getBitWidth(); ++i)
        ret += (unsigned)hasBit(i);
    return ret;
}

void Bits::bitOp(const Bits &a, const Bits &b, void(*fun)(bool,bool,bool&,bool&))
{
    CANAL_ASSERT(a.getBitWidth() == b.getBitWidth());
    for (unsigned pos = 0; pos < a.getBitWidth(); ++pos)
    {
        bool b0 = a.getBit(pos), b1 = b.getBit(pos);
        fun(a.hasBit(pos), b.hasBit(pos), b0, b1);
        if (b0)
            setBit(pos, 0);
        if (b1)
            setBit(pos, 1);
    }
}

} // namespace Integer
} // namespace Canal
