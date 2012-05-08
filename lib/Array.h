#ifndef LIBCANAL_ARRAY_H
#define LIBCANAL_ARRAY_H

#include "Value.h"

namespace Canal {
namespace Array {

// We treat all array members as a single value.  This means, that all
// the operations on the array are merged and used to move the single
// value up in its lattice.
class SingleItem : public Value
{
public:
    Value *mValue;

    // Number of elements in the array.
    // It is either a Constant or Integer::Container.
    Value *mSize;

public:
    SingleItem();
    SingleItem(const SingleItem &singleItem);
    virtual ~SingleItem();

    // Implementation of Value::clone().
    // Covariant return type.
    virtual SingleItem *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_H
