#include "IntegerBits.h"

namespace Canal {
namespace Integer {

Bits::Bits() : mBits0(0), mBits1(0)
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
    return new Bits<T>(*this);
}

void Bits::merge(const Value &value)
{
    const Bits &other = dynamic_cast<const Bits&>(value);
    mBits0 |= other.mBits0;
    mBits1 |= other.mBits1;
}

float Bits::accuracy() const
{
    int allBits = sizeof(T) * 8;
    return 1.0 - (this->bitcount() / (float)allBits);
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
    for (int pos = 0; pos < sizeof(T) * 8; ++pos)
    {
        if (hasBit(pos))
            ostream << getBit(pos);
        else
            ostream << (this->getBit(pos) ? '_' : '?');
    }
}

void Bits::and_(const Value &a, const Value &b)
{
    const Bits &aa = dynamic_cast<const Bits&>(a),
        &bb = dynamic_cast<const Bits&>(b);

    // mBits0 = aa.mBits0 & bb.mBits0;
    // mBits1 = aa.mBits1 & bb.mBits1;
    bitOp(aa, bb, bitAnd);
}

void Bits::or_(const Value &a, const Value &b)
{
    const Bits &aa = dynamic_cast<const Bits&>(a),
        &bb = dynamic_cast<const Bits&>(b);

    // mBits0 = aa.mBits0 | bb.mBits0;
    // mBits1 = aa.mBits1 | bb.mBits1;
    bitOp(aa, bb, bitOr);
}

void Bits::xor_(const Value &a, const Value &b)
{
    const Bits &aa = dynamic_cast<const Bits&>(a),
        &bb = dynamic_cast<const Bits&>(b);

    // First digit is mBits1, second is mBits0
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
    bitOp(a1, a2, bitXor);
}

bool Bits::hasBit(const unsigned int pos) const
{
    return (this->Bits0 & (1 << pos)) xor (this->Bits1 & (1 << pos));
}

bool Bits::getBit(const unsigned int pos) const
{
    return (this->Bits0 & (1 << pos) ? 0 : 1);
}

void Bits::setBit(const unsigned int pos, const bool bit)
{
    T &where = (bit ? this->Bits1 : this->Bits0);
    T &other = (bit ? this->Bits0 : this->Bits1);
    where |= 1 << pos;
    other &= ~(1 << pos);
}

unsigned Bits::bitcount() const
{
    unsigned ret = 0;
    for (unsigned i = 0; i < sizeof(T) * 8; i ++)
        ret += (unsigned)hasBit(i);
    return ret;
}

//Functions for bitwise operations
#define SET0 bit0 = 1; bit1 = 0
#define SET1 bit0 = 0; bit1 = 1
#define SETBOTH bit0 = 1; bit1 = 1
#define BITHEADER bool know1, bool know2, bool &bit0, bool &bit1
#define BITCOUNT (unsigned int) know1 + (unsigned int) know2

  static void bitAnd(BITHEADER) {
      switch (BITCOUNT)
      {
      case 2:
          if (bit0 && bit1) { SET0; }
          else { SET1; }
          break;
      case 1:
          if ((know1 && !bit0) || (know2 && !bit1)) { // One of the bits is 0, so the result is 0
              SET0;
          }
          else { SETBOTH; }
          break;
      default: //No info
          SETBOTH;
          break;
      }
  }

  static void bitXor(BITHEADER) {
      switch (BITCOUNT)
      {
      case 2:
          if (bit0 xor bit1) { SET1; }
          else { SET0; }
          break;
      default: //No info
          SETBOTH;
      }
  }

  static void bitOr(BITHEADER) {
      switch (BITCOUNT)
      {
      case 2:
          if (bit0 || bit1) { SET1; }
          else { SET0; }
          break;
      case 1:
          if ((know1 && bit0) || (know2 && bit1)) { // One of the bits is 1, so the result is 1
              SET1;
          }
          else { SETBOTH; }
          break;
      default: //No info
          SETBOTH;
      }
  }

#undef BITHEADER
#undef BITCOUNT
#undef SET0
#undef SET1
#undef SETBOTH

  template <typename F>
  void bitOp(const Bits<T> &a, const Bits<T> &b, F f) {
      for (unsigned int pos = 0; pos < sizeof(T) * 8; pos ++) {
          bool b0 = a.getBit(pos), b1 = b.getBit(pos);
          f(a.hasBit(pos), b.hasBit(pos), b0, b1);
          if (b0) this->setBit(pos, 0);
          if (b1) this->setBit(pos, 1);
      }
  }


} // namespace Integer
} // namespace Canal
