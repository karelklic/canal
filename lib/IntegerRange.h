#ifndef CANAL_INTEGER_RANGE_H
#define CANAL_INTEGER_RANGE_H

#include "Value.h"
#include <llvm/Constants.h>

namespace Canal {
namespace Integer {

// Abstracts integer values as a range min - max.
template <typename T>
class Range : public Value
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

} // namespace Integer
} // namespace Canal

#endif
