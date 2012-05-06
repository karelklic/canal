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

  //Unsigned
  bool UTop;
  llvm::APInt UFrom;
  llvm::APInt UTo;

  //Signed
  bool STop;
  llvm::APInt SFrom;
  llvm::APInt STo;

  enum Declared { UNSIGNED, SIGNED, BOTH };

  //Which type is declared
  Declared declared;

 public:
  // Initializes to the lowest value.
  Range() : Empty(true) {}

  // Covariant return type -- overrides AbstractValue::clone().
  virtual Range *clone() const
  {
    return new Range(*this);
  }

  Range(const llvm::APInt &constant) : Empty(false), declared(BOTH),
      UTop(false), UFrom(constant), UTo(constant),
      STop(false), SFrom(constant), STo(constant) {// : AbstractValue(constant) {
  }

  virtual bool operator==(const AbstractValue& rhs) const {
      const Range& other = (const Range&) rhs;
      return this->Empty == other.Empty &&
              this->declared == other.declared &&
              this->UTop == other.UTop &&
              this->UFrom == other.UFrom &&
              this->UTo == other.UTo &&
              this->STop == other.STop &&
              this->SFrom == other.SFrom &&
              this->STo == other.STo;
  }

  virtual void merge(const AbstractValue &v) {
      const Range& other = (const Range&) v;
      this->Empty = other.Empty;
      if (this->Empty) return;
      //TODO - what if only one is defined and both in the other?

      /*
      if (this->Top || other.Top) {
          this->Top = true;
      }
      else {
        this->setLower(other);
        this->setHigher(other);
      }*/
  }

  virtual size_t memoryUsage() const {
      return 4 * sizeof(llvm::APInt) + 3 * sizeof(bool) + sizeof(Declared);
  }

  virtual bool limitMemoryUsage(size_t size) {
      return false;
  }

  virtual llvm::APInt Srange() const {
      if (this->SFrom.sgt(this->STo)) { //Inverted
          //Interval 9-0 (in mod 10, two values (0 and 9))
          //Max Value -> 9
          //Min Value -> 0
          return (llvm::APInt::getSignedMaxValue(this->SFrom.getBitWidth()) - this->STo) +
                  (llvm::APInt::getSignedMinValue(this->STo.getBitWidth()) - this->SFrom) + 2;
      }
      else {
          //Interval 0-0 -> one value (0)
          return this->STo - this->SFrom + 1;
      }
  }

  virtual llvm::APInt Urange() const {
      if (this->SFrom.ugt(this->UTo)) { //Inverted
          //Interval 9-0 (in mod 10, two values (0 and 9))
          //Max Value -> 9
          //Min Value -> 0
          return (llvm::APInt::getMaxValue(this->UFrom.getBitWidth()) - this->UTo) +
                  (llvm::APInt::getMinValue(this->UTo.getBitWidth()) - this->UFrom) + 2;
      }
      else {
          //Interval 0-0 -> one value (0)
          return this->UTo - this->UFrom + 1;
      }
  }

  virtual float accuracy() const {
      //TODO - what if only one is set given?
      /*
      if (this->Top) return 0;
      else if(this->From == this->To) {
          return 1.0;
      }
      else {
          return 0.5; //Will probably need APFloat probably for accuracy
          //Pseudocode:
          //return 1.0 - this->range() / (this->getMaximumSignedValue(this->From.getBitWidth()) - this->getMinimumSignedValue(this->From.getBitWidth()));
      }
      */
      /*return 1.0; /* - (
                  (this->Infinity ? std::numeric_limits<llvm::APInt>::max() : this->To) -
                  (this->NegativeInfinity ? std::numeric_limits<llvm::APInt>::min() : this->From)
              ) / (std::numeric_limits<llvm::APInt>::max() - std::numeric_limits<llvm::APInt>::min());*/
      //TODO
  }

  virtual bool isBottom() const {
      return this->Empty;
  }

  virtual void setTop() {
      if (this->declared == SIGNED || this->declared == BOTH) this->STop = true;
      if (this->declared == UNSIGNED || this->declared == BOTH) this->UTop = true;
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
      this->Empty = false;

      if (a1.declared == SIGNED || a1.declared == BOTH &&
              a2.declared == SIGNED || a2.declared == BOTH) {//Signed part
          if (a1.STop || a2.STop) {
              this->STop = true;
              return;
          }

          this->SFrom = a1.SFrom + a2.SFrom;
          this->STo = a1.STo + a2.STo;

          if (Soverflow(a1, a2)) {
              this->STop = true;
          }
      }

      if (a1.declared == UNSIGNED || a1.declared == BOTH &&
              a2.declared == UNSIGNED || a2.declared == BOTH) {//Unsigned part
          if (a1.UTop || a2.UTop) {
              this->UTop = true;
              return;
          }

          this->UFrom = a1.UFrom + a2.UFrom;
          this->UTo = a1.UTo + a2.UTo;

          if (Uoverflow(a1, a2)) {
              this->UTop = true;
          }
      }
  }

  void sub(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->Empty = false;

      if (a1.declared == SIGNED || a1.declared == BOTH &&
              a2.declared == SIGNED || a2.declared == BOTH) {//Signed part
          if (a1.STop || a2.STop) {
              this->STop = true;
              return;
          }

          this->SFrom = a1.SFrom - a2.STo;
          this->STo = a1.STo - a2.SFrom;

          if (Soverflow(a1, a2)) {
              this->STop = true;
          }
      }

      if (a1.declared == UNSIGNED || a1.declared == BOTH &&
              a2.declared == UNSIGNED || a2.declared == BOTH) {//Unsigned part
          if (a1.UTop || a2.UTop) {
              this->UTop = true;
              return;
          }

          this->UFrom = a1.UFrom - a2.UTo;
          this->UTo = a1.UTo - a2.UFrom;

          if (Uoverflow(a1, a2)) {
              this->UTop = true;
          }
      }
  }

  void smul(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->Empty = false;

      if (a1.declared == SIGNED || a1.declared == BOTH &&
              a2.declared == SIGNED || a2.declared == BOTH) { //Both must be signed

          this->declared = SIGNED;

          if (a1.STop || a2.STop) {
              this->STop = true;
              return;
          }

          //TODO - overflow
          this->SFrom = Smin(a1.SFrom * a2.SFrom, a1.STo * a2.STo, a1.SFrom * a2.STo, a1.STo * a2.SFrom);
          this->STo = Smax(a1.SFrom * a2.SFrom, a1.STo * a2.STo, a1.SFrom * a2.STo, a1.STo * a2.SFrom);
      }
      else { //If both are not signed
          this->setTop();
      }
  }

  void sdiv(const AbstractValue &a, const AbstractValue &b) {
      const Range &a1 = dynamic_cast<const Range &> (a), &a2 = dynamic_cast<const Range &> (b);
      this->Empty = false;

      if (a1.declared == SIGNED || a1.declared == BOTH &&
              a2.declared == SIGNED || a2.declared == BOTH) { //Both must be signed

          this->declared = SIGNED;

          if (a1.STop || a2.STop) {
              this->STop = true;
              return;
          }
          this->SFrom = Smin(a1.SFrom. CMP(div) (a2.SFrom), a1.STo. CMP(div) (a2.STo), a1.SFrom. CMP(div) (a2.STo), a1.STo. CMP(div) (a2.SFrom));
          this->STo = Smax(a1.SFrom. CMP(div) (a2.SFrom), a1.STo. CMP(div) (a2.STo), a1.SFrom. CMP(div) (a2.STo), a1.STo. CMP(div) (a2.SFrom));
      }
      else { //If both are not signed
          this->setTop();
      }
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

#define MIN2U(a, b) ( a.ult(b) ? a : b )
#define MAX2U(a, b) ( a.ugt(b) ? a : b )
#define MIN2S(a, b) ( a.slt(b) ? a : b )
#define MAX2S(a, b) ( a.sgt(b) ? a : b )

//For merging
/*
  void setLower(const Range& other) {
      //TODO - Signed or Unsigned
      this->From = MIN2(this->From, other.From);
  }

  void setHigher(const Range& other) {
      //TODO - Signed or Unsigned
      this->To = MAX2(this->To, other.To);
  }
*/

  static const llvm::APInt& Smin(const llvm::APInt& a, const llvm::APInt& b, const llvm::APInt& c, const llvm::APInt& d) {
      return MIN2S ( (MIN2S(a, b)), (MIN2S(c, d)) );
  }

  static const llvm::APInt& Umin(const llvm::APInt& a, const llvm::APInt& b, const llvm::APInt& c, const llvm::APInt& d) {
      return MIN2U ( (MIN2U(a, b)), (MIN2U(c, d)) );
  }

  static const llvm::APInt& Smax(const llvm::APInt& a, const llvm::APInt& b, const llvm::APInt& c, const llvm::APInt& d) {
      return MAX2S ( (MAX2S(a, b)), (MAX2S(c, d)) );
  }
  static const llvm::APInt& Umax(const llvm::APInt& a, const llvm::APInt& b, const llvm::APInt& c, const llvm::APInt& d) {
      return MAX2U ( (MAX2U(a, b)), (MAX2U(c, d)) );
  }

  //Check for overflow in addition and substraction
  static bool Soverflow(const Range& a, const Range& b) {
      //llvm::APInt r1 = (a.To - a.From) + 1;
      //llvm::APInt r2 = (b.To - b.From) + 1;
      bool res;
      a.Srange().smul_ov(b.Srange(), res);
      return res;
  }

  //Check for overflow in addition and substraction
  static bool Uoverflow(const Range& a, const Range& b) {
      //llvm::APInt r1 = (a.To - a.From) + 1;
      //llvm::APInt r2 = (b.To - b.From) + 1;
      bool res;
      a.Urange().umul_ov(b.Urange(), res);
      return res;
  }
};

} // namespace AbstractInteger

#endif
