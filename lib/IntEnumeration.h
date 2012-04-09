#ifndef CANAL_INT_ENUMERATION_H
#define CANAL_INT_ENUMERATION_H

#include "AbstractValue.h"
#include <llvm/Constants.h>
#include <set>

namespace AbstractInteger {

template <typename T>
class Enumeration : public AbstractValue
{
 public:
  std::set<T> Values;

 public:
  // Covariant return type -- overrides AbstractValue::clone().
  virtual Enumeration<T> *clone() const
  {
    return new Enumeration<T>(*this);
  }

  //Initialize const value
  Enumeration(const llvm::Constant* constant) : AbstractValue(constant) {
      //TODO
  }

  virtual bool operator==(const AbstractValue& rhs) const {
      const Enumeration& other = (const Enumeration&) rhs;
      if (this->Values.size() != other.Values.size()) return false;
      //TODO - get this done when Enumeration is not template class
      //for (std::set<T>::const_iterator it = this->Values.begin(), it2 = other.Values.begin();
      //     it != this->Values.end(); it ++, it2 ++) {
      //    if (*it != *it2) return false;
      //}
      return true;
  }
};

} // namespace AbstractInteger

#endif
