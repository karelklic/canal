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
};

} // namespace AbstractInteger

#endif
