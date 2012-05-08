#ifndef LIBCANAL_INTEGER_RANGE_H
#define LIBCANAL_INTEGER_RANGE_H

#include "Value.h"
#include <llvm/Constants.h>

namespace llvm {
    class APInt;
}

namespace Canal {
namespace Integer {

// Abstracts integer values as a range min - max.
class Range : public Value
{
public:
    bool mEmpty;
    bool mTop;
    llvm::APInt mFrom;
    llvm::APInt mTo;

public:
    // Initializes to the lowest value.
    Range(unsigned numBits);
    Range(const llvm::APInt &constant);

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Range *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;

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

#endif // LIBCANAL_INTEGER_RANGE_H
