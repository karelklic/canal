#ifndef CANAL_INT_ENUMERATION_H
#define CANAL_INT_ENUMERATION_H

#include "AbstractValue.h"
#include <llvm/Constants.h>
#include <set>
#include <llvm/ADT/APInt.h>

namespace AbstractInteger {

class APIntSetHack : public llvm::APInt {
public:
    APIntSetHack (const llvm::APInt& other) : llvm::APInt(other) {}

    bool operator<(const APIntSetHack& other) const {
        return this->slt(other);
    }
};

//Maximum of items in set
#define SIZE_LIMIT 20

class Enumeration : public AbstractValue
{
 public:
  std::set<APIntSetHack> Values;
  bool Top;

 public:
  //Initializes to the lowest value
  Enumeration() : Top(false) {}

  /*Enumeration(const Enumeration& other) {
      this->Top = other.Top;
      if (this->Top) return; //No need to copy items if Top is set
      for (std::set<llvm::APInt>::const_iterator it = other.Values.begin(); it != other.Values.end(); it ++) {
          this->Values.insert(*it);
      }
  }*/

  // Covariant return type -- overrides AbstractValue::clone().
  virtual Enumeration *clone() const
  {
    return new Enumeration(*this);
  }

  //Initialize const value
  Enumeration(const llvm::APInt& constant) : Top(false) { // : AbstractValue(constant) {
      this->Values.insert(APIntSetHack(constant));
  }

  virtual bool operator==(const AbstractValue& rhs) const {
      const Enumeration& other = dynamic_cast<const Enumeration&> (rhs);
      return this->Top == other.Top && this->Values == other.Values;
  }

  virtual void merge(const AbstractValue &v) {
      const Enumeration& other = dynamic_cast<const Enumeration&> (v);
      if (this->Top || other.Top) {
          this->setTop();
          return;
      }
      else {
          this->Values.insert(other.Values.begin(), other.Values.end());
          this->limit();
      }
  }

  virtual size_t memoryUsage() const {
      if (this->Top) {
          return sizeof(bool) + sizeof(std::set<APIntSetHack>);
      }
      else {
          //Approximation
          return sizeof(bool) + sizeof(std::set<APIntSetHack>) + sizeof(APIntSetHack) * this->Values.size();
      }
  }

  virtual bool limitMemoryUsage(size_t size) {
      if (this->Top) return false;
      else {
          setTop();
          return true;
      }
  }

  virtual float accuracy() const {
      if (this->Top) return 0;
      if (this->Values.size() == 1) return 1;
      else {
          return 0.5;
          //TODO - get range of APInt in set and divide it by number of items in set
      }
  }

  virtual bool isBottom() const {
      return !this->Top && this->Values.size() == 0;
  }

  virtual void setTop() {
      this->Top = true;
      this->Values.clear();
  }

  virtual void printToStream(llvm::raw_ostream &ostream) const {
      //TODO - raw_ostream string support
      /*if (this->Top) {
          ostream << "Top value";
      }
      else {
          ostream << '{';
          for (std::set<APIntSetHack>::const_iterator it = this->Values.begin(); it != this->Values.end(); it ++) {
              if (it != this->Values.begin()) ostream << ", ";
              ostream << *it;
          }
          ostream << '}';
      }*/
  }

#define METHOD(classname, operation) \
  void classname(const AbstractValue &a, const AbstractValue &b) { \
    const Enumeration &l = dynamic_cast<const Enumeration&>(a), &r = dynamic_cast<const Enumeration&>(b); \
    if (l.Top || r.Top) { this->setTop(); return; } \
    /*TODO - can this be extracted to function?*/ \
    for (std::set<APIntSetHack>::const_iterator lit = l.Values.begin(); lit != l.Values.end(); lit ++) { \
        for (std::set<APIntSetHack>::const_iterator rit = r.Values.begin(); rit != r.Values.end(); rit ++) { \
            this->Values.insert((*lit).operation(*rit)); \
            if (this->limit()) return; \
        } \
    } \
  }
#define METHOD_OV(classname, operation) \
  void classname(const AbstractValue &a, const AbstractValue &b) { \
    const Enumeration &l = dynamic_cast<const Enumeration&>(a), &r = dynamic_cast<const Enumeration&>(b); \
    if (l.Top || r.Top) { this->setTop(); return; } \
    /*TODO - can this be extracted to function?*/ \
    for (std::set<APIntSetHack>::const_iterator lit = l.Values.begin(); lit != l.Values.end(); lit ++) { \
        for (std::set<APIntSetHack>::const_iterator rit = r.Values.begin(); rit != r.Values.end(); rit ++) { \
            bool overflow; \
            this->Values.insert((*lit).operation(*rit, overflow)); \
            if (overflow) { this->setTop(); return; } \
            if (this->limit()) return; \
        } \
    } \
  }


  METHOD(add, operator+)
  METHOD(sub, operator-)
  METHOD_OV(smul, smul_ov)
  METHOD_OV(umul, umul_ov)
  METHOD_OV(sdiv, sdiv_ov)
  METHOD(udiv, udiv)
  METHOD(urem, urem)
  METHOD(srem, srem)
  METHOD(shl, shl)
  METHOD(ashr, ashr)
  METHOD(lshr, lshr)
  METHOD(and_, operator&)
  METHOD(or_, operator|)
  METHOD(xor_, operator^)

private:
  inline bool limit() {
      if (this->Values.size() > SIZE_LIMIT) {
          this->setTop();
          return true;
      }
      return false;
  }
};

} // namespace AbstractInteger

#endif

