#ifndef CANAL_INTEGER_ENUMERATION_H
#define CANAL_INTEGER_ENUMERATION_H

#include "Value.h"
#include <llvm/ADT/APInt.h>
#include <set>

namespace Canal {
namespace Integer {

class Enumeration : public Value
{
 public:
    std::set<llvm::APInt> Values;

 public:
  // Covariant return type -- overrides AbstractValue::clone().
  virtual Enumeration *clone() const
  {
    return new Enumeration(*this);
  }
};

} // namespace Integer
} // namespace Canal

#endif
