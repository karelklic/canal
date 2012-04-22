#ifndef CANAL_INTEGER_RANGE_H
#define CANAL_INTEGER_RANGE_H

#include "Value.h"
#include <llvm/Constants.h>

namespace Canal {
namespace Integer {

// Abstracts integer values as a range min - max.
class Range : public Value
{
public:
    bool mEmpty;

    llvm::APInt mFrom;
    bool mFromInfinity;

    llvm::APInt mTo;
    bool mToInfinity;

public:
    // Initializes to the lowest value.
    Range(unsigned numBits);

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
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;

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

#endif
