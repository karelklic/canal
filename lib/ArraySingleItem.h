#ifndef LIBCANAL_ARRAY_SINGLE_ITEM_H
#define LIBCANAL_ARRAY_SINGLE_ITEM_H

#include "Value.h"
#include "Array.h"

namespace Canal {
namespace Array {

// The most trivial array type.  It treats all array members as a
// single value.  This means all the operations on the array are
// merged and used to move the single value up in its lattice.
//
// This array type is very imprecise.
class SingleItem : public Value, public Array
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

public: // Implementation of Value.
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

public: // Implementation of Array.
    // Implementation of Array::get().
    virtual Value *get(const Value &offset) const;
    // Implementation of Array::set().
    virtual void set(const Value &offset, const Value &value);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_SINGLE_ITEM_H
