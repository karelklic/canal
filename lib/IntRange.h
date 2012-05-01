#ifndef CANAL_INT_RANGE_H
#define CANAL_INT_RANGE_H

#include "AbstractValue.h"
#include <llvm/Constants.h>
#include <llvm/ADT/APInt.h>
#include <limits>

#define CMP(OP) s ## OP

namespace AbstractInteger {

// Abstracts integer values as a range min - max.
class Range : public AbstractValue
{
 public:
  bool Empty;

  bool NegativeInfinity;
  llvm::APInt From;

  bool Infinity;
  llvm::APInt To;

 public:
  // Initializes to the lowest value.
  Range() : Infinity(false), NegativeInfinity(false), Empty(true) {}

  // Covariant return type -- overrides AbstractValue::clone().
  virtual Range *clone() const
  {
    return new Range(*this);
  }

  Range(const llvm::APInt &constant) : From(constant), To(constant) {// : AbstractValue(constant) {

  }

  virtual bool operator==(const AbstractValue& rhs) const {
      const Range& other = (const Range&) rhs;
      return this->Empty == other.Empty &&
             this->NegativeInfinity == other.NegativeInfinity &&
              this->Infinity == other.Infinity &&
              this->From == other.From &&
              this->To == other.To;
  }

  virtual void merge(const AbstractValue &v) {
      const Range& other = (const Range&) v;
      this->setLower(other);
      this->setHigher(other);
  }

  virtual size_t memoryUsage() const {
      return 2 * sizeof(llvm::APInt) + 3 * sizeof(bool);
  }

  virtual bool limitMemoryUsage(size_t size) {
      return false;
  }

  virtual float accuracy() const {
      return 1.0;/* - (
                  (this->Infinity ? std::numeric_limits<llvm::APInt>::max() : this->To) -
                  (this->NegativeInfinity ? std::numeric_limits<llvm::APInt>::min() : this->From)
              ) / (std::numeric_limits<llvm::APInt>::max() - std::numeric_limits<llvm::APInt>::min());*/
      //TODO
  }

  virtual bool isBottom() const {
      return this->Empty;
  }

  virtual void setTop() {
      this->Infinity = true;
      this->NegativeInfinity = true;
  }

  virtual void printToStream(llvm::raw_ostream &ostream) const {
      //TODO - raw_ostream string support
      //if (this->NegativeInfinity) ostream << llvm::StringRef("(-Infinity");
      //else ostream << "<" << this->From;
      //ostream << ":";
      //if (this->Infinity) ostream << "Infinity)";
      //else ostream << this->To << ">";
  }

  void add(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->From = a1.From + a2.From;
      this->To = a1.To + a2.To;
  }

  void sub(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->From = a1.From - a2.To;
      this->To = a1.To - a2.From;
  }

  void mul(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->From = min(a1.From * a2.From, a1.To * a2.To, a1.From * a2.To, a1.To * a2.From);
      this->To = max(a1.From * a2.From, a1.To * a2.To, a1.From * a2.To, a1.To * a2.From);
  }

  void div(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->From = min(a1.From. CMP(div) (a2.From), a1.To. CMP(div) (a2.To), a1.From. CMP(div) (a2.To), a1.To. CMP(div) (a2.From));
      this->To = max(a1.From. CMP(div) (a2.From), a1.To. CMP(div) (a2.To), a1.From. CMP(div) (a2.To), a1.To. CMP(div) (a2.From));
  }

  /*void add(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->binaryOp(a, b, Range::plus);
  }

  void sub(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->binaryOpInverted(a, b, Range::minus);
  }

  void mul(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->binaryOp(a, b, Range::multiple);
  }

  void div(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->binaryOp(a, b, Range::divide);
  }*/

protected:

#define MIN2(a, b) ( a. CMP(lt) (b) ? a : b )
#define MAX2(a, b) ( a. CMP(gt) (b) ? a : b )

//For merging
  void setLower(const Range& other) {
      if (this->NegativeInfinity || other.NegativeInfinity) {
          this->NegativeInfinity = true;
      }
      else {
          //TODO - Signed or Unsigned
          this->From = MIN2(this->From, other.From);
      }
  }

  void setHigher(const Range& other) {
      if (this->Infinity || other.Infinity) {
          this->Infinity = true;
      }
      else {
          //TODO - Signed or Unsigned
          this->To = MAX2(this->To, other.To);
      }
  }

  static const llvm::APInt& min(const llvm::APInt& a, const llvm::APInt& b, const llvm::APInt& c, const llvm::APInt& d) {
      return MIN2 ( (MIN2(a, b)), (MIN2(c, d)) );
  }

  static const llvm::APInt& max(const llvm::APInt& a, const llvm::APInt& b, const llvm::APInt& c, const llvm::APInt& d) {
      return MAX2 ( (MAX2(a, b)), (MAX2(c, d)) );
  }

/*  llvm::APInt plus(const llvm::APInt& a, const llvm::APInt& b) const {
      return a + b;
  }

  llvm::APInt minus(const llvm::APInt& a, const llvm::APInt& b) const {
      return a - b;
  }

  llvm::APInt multiple(const llvm::APInt& a, const llvm::APInt& b) const {
      return a * b;
  }

  llvm::APInt divide(const llvm::APInt& a, const llvm::APInt& b) const {
      return a * b;
  }

  template <typename F>
  void binaryOp2(const Range &a, const Range& b, F f) {
      if (a.NegativeInfinity || b.NegativeInfinity) this->NegativeInfinity = true;
      else {
          this->From = F(a.From, b.From);
      }
      if (a.Infinity || b.Infinity) this->Infinity = true;
      else {
          this->To = F(a.To, b.To);
      }
  }

  template <typename F>
  void binaryOp2Inverted(const Range &a, const Range& b, F f) {
      if (a.NegativeInfinity || b.NegativeInfinity) this->NegativeInfinity = true;
      else {
          this->From = F(a.From, b.To);
      }
      if (a.Infinity || b.Infinity) this->Infinity = true;
      else {
          this->To = F(a.To, b.From);
      }
  }*/
};

} // namespace AbstractInteger

#endif
