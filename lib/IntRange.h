#ifndef CANAL_INT_RANGE_H
#define CANAL_INT_RANGE_H

#include "AbstractValue.h"
#include <llvm/Constants.h>
#include <limits>

namespace AbstractInteger {

// Abstracts integer values as a range min - max.
template <typename T>
class Range : public AbstractValue
{
 public:
  bool Empty;

  bool NegativeInfinity;
  T From;

  bool Infinity;
  T To;

 public:
  // Initializes to the lowest value.
  Range() : Infinity(false), NegativeInfinity(false), Empty(true) {}

  // Covariant return type -- overrides AbstractValue::clone().
  virtual Range<T> *clone() const
  {
    return new Range<T>(*this);
  }

  Range(const llvm::Constant* constant) : AbstractValue(constant) {
      //TODO
  }

  virtual bool operator==(const AbstractValue& rhs) const {
      const Range& other = (const Range&) rhs;
      return this->Empty == other.Empty &&
             this->NegativeInfinity == other.NegativeInfinity &&
              this->Infinity == other.Infinity &&
              this->From = other.From &&
              this->To = other.To;
  }

  virtual void merge(const AbstractValue &v) {
      const Range& other = (const Range&) v;
      this->setLower(other);
      this->setHigher(other);
  }

  virtual size_t memoryUsage() const {
      return 2 * sizeof(T) + 3 * sizeof(bool);
  }

  virtual bool limitMemoryUsage(size_t size) {
      return false;
  }

  virtual float accuracy() const {
      return 1.0 - (
                  (this->Infinity ? std::numeric_limits<T>::max() : this->To) -
                  (this->NegativeInfinity ? std::numeric_limits<T>::min() : this->From)
              ) / (std::numeric_limits<T>::max() - std::numeric_limits<T>::min());
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
      const Range<T> &a1 = dynamic_cast<const Range<T> &> (a), &a2 = dynamic_cast<const Range<T> &> (b);
      this->binaryOp(a, b, Range::plus);
  }

  void sub(const AbstractValue &a, const AbstractValue &b) {
      const Range<T> &a1 = dynamic_cast<const Range<T> &> (a), &a2 = dynamic_cast<const Range<T> &> (b);
      this->binaryOp(a, b, Range::minus);
  }

  void mul(const AbstractValue &a, const AbstractValue &b) {
      const Range<T> &a1 = dynamic_cast<const Range<T> &> (a), &a2 = dynamic_cast<const Range<T> &> (b);
      this->binaryOp(a, b, Range::multiple);
  }

  void div(const AbstractValue &a, const AbstractValue &b) {
      const Range<T> &a1 = dynamic_cast<const Range<T> &> (a), &a2 = dynamic_cast<const Range<T> &> (b);
      this->binaryOp(a, b, Range::divide);
  }

protected:
  void setLower(const Range& other) {
      if (this->NegativeInfinity || other.NegativeInfinity) {
          this->NegativeInfinity = true;
      }
      else {
          this->From = (this->From < other.From ? this->From : other.From);
      }
  }

  void setHigher(const Range& other) {
      if (this->Infinity || other.Infinity) {
          this->Infinity = true;
      }
      else {
          this->To = (this->To < other.To ? this->To : other.To);
      }
  }

  T plus(const T& a, const T& b) const {
      return a + b;
  }

  T minus(const T& a, const T& b) const {
      return a - b;
  }

  T multiple(const T& a, const T& b) const {
      return a * b;
  }

  T divide(const T& a, const T& b) const {
      return a * b;
  }

  template <typename F>
  void binaryOp(const Range<T> &a, const Range<T>& b, F f) {
      if (a.NegativeInfinity || b.NegativeInfinity) this->NegativeInfinity = true;
      else {
          this->From = F(a.From, b.From);
      }
      if (a.Infinity || b.Infinity) this->Infinity = true;
      else {
          this->To = F(a.To, b.To);
      }
  }
};

} // namespace AbstractInteger

#endif
