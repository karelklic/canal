#ifndef LIBCANAL_INTEGER_ENUMERATION_H
#define LIBCANAL_INTEGER_ENUMERATION_H

#include "Value.h"
#include <llvm/ADT/APInt.h>
#include <set>

namespace Canal {
namespace Integer {

struct APIntComp
{
  bool operator()(const llvm::APInt &a, const llvm::APInt &b) const
  {
      return a.ult(b);
  }
};

class Enumeration : public Value, public AccuracyValue
{
public:
    std::set<llvm::APInt, APIntComp> mValues;
    bool mTop;

public:
    // Initializes to the lowest value.
    Enumeration();

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Enumeration *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString() const;

public: // Implementation of AccuracyValue.
    // Implementation of AccuracyValue::accuracy().
    virtual float accuracy() const;
    // Implementation of AccuracyValue::isBottom().
    virtual bool isBottom() const;
    // Implementation of AccuracyValue::setTop().
    virtual void setTop();
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_ENUMERATION_H
