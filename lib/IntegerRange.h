#ifndef CANAL_INT_RANGE_H
#define CANAL_INT_RANGE_H

#include "AbstractValue.h"
#include <llvm/Constants.h>

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
};

} // namespace AbstractInteger

#endif
