#ifndef CANAL_INT_BITS_H
#define CANAL_INT_BITS_H

#include "AbstractValue.h"
#include <llvm/Constants.h>

namespace AbstractInteger {

// Abstracts integers as a bitfield.
template <typename T>
class Bits : public AbstractValue
{
 public:
  // When a bit in Bits0 is 1, the value is known to contain zero at
  // this position.
  T Bits0;
  // When a bit in Bits1 is 1, the value is known to contain one at
  // this position.
  T Bits1;

 public:
  // Initializes to the lowest value.
  Bits() : Bits0(0), Bits1(0) {}

  //Initialize const value
  Bits(const llvm::Constant* constant) : AbstractValue(constant) {
      //TODO
  }

  virtual bool operator==(const AbstractValue& rhs) const {
      const Bits& other = (const Bits&) rhs;
      for (unsigned int pos = 0; pos < sizeof(T) * 8; pos ++) {
          if (this->hasBit(pos)) {
              if (! other.hasBit(pos)) return false;
              if (this->getBit(pos) != other.getBit(pos)) return false;
          }
      }
      return true;
  }

  // Covariant return type -- overrides AbstractValue::clone().
  virtual Bits<T> *clone() const
  {
    return new Bits<T>(*this);
  }

  virtual void merge(const AbstractValue &V)
  {
    const Bits<T> &other = dynamic_cast<const Bits<T> &> (V);
    Bits0 |= other.Bits0;
    Bits1 |= other.Bits1;
  }

  virtual size_t memoryUsage() const
  {
    return 2 * sizeof(T);
  }

  virtual bool limitMemoryUsage(size_t size)
  {
    // memory usage of this value cannot be lowered
    return false;
  }

  virtual float accuracy() const
  {
    int allBits = sizeof(T) * 8;
    return 1.0 - (this->bitcount() / (float)allBits);
  }

  virtual bool isBottom() const {
      return this->Bits0 == 0 && this->Bits1 == 0;
  }

  virtual void setTop() {
      this->Bits0 = ~0;
      this->Bits1 = ~0;
  }

  virtual void printToStream(llvm::raw_ostream &ostream) const {
      for (unsigned int pos = 0; pos < sizeof(T) * 8; pos ++) {
          if (this->hasBit(pos)) {
              ostream << this->getBit(pos);
          }
          else {
              ostream << (this->getBit(pos) ? '_' : '?');
          }
      }
  }

  void and_ (const AbstractValue &a, const AbstractValue &b)
  {
      const Bits<T> &a1 = dynamic_cast<const Bits<T> &> (a), &a2 = dynamic_cast<const Bits<T> &> (b);
      this->bitOp(a1, a2, bitAnd);
  }

  void or_ (const AbstractValue &a, const AbstractValue &b)
  {
      const Bits<T> &a1 = dynamic_cast<const Bits<T> &> (a), &a2 = dynamic_cast<const Bits<T> &> (b);
      this->bitOp(a1, a2, bitOr);
  }

  void xor_ (const AbstractValue &a, const AbstractValue &b)
  {
      const Bits<T> &a1 = dynamic_cast<const Bits<T> &> (a), &a2 = dynamic_cast<const Bits<T> &> (b);
      this->bitOp(a1, a2, bitXor);
  }

  bool hasBit(const unsigned int pos) const {
      return (this->Bits0 & (1 << pos)) xor (this->Bits1 & (1 << pos));
  }

  //If this bit is set (hasBit(pos) == true), it will return its value
  //Otherwise it will return 0 if it can represent both values, 1 if unset
  bool getBit(const unsigned int pos) const {
      return (this->Bits0 & (1 << pos) ? 0 : 1);
  }

  void setBit(const unsigned int pos, const bool bit) {
      T &where = (bit ? this->Bits1 : this->Bits0);
      T &other = (bit ? this->Bits0 : this->Bits1);
      where |= 1 << pos;
      other &= ~(1 << pos);
  }

private:

  unsigned int bitcount() const { //Number of definately known bits
      unsigned int ret = 0;
      for (unsigned int i = 0; i < sizeof(T) * 8; i ++) {
          ret += (unsigned int) this->hasBit(i);
      }
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
};

} // namespace AbstractInteger

#endif
