#ifndef LIBCANAL_ARRAY_SINGLE_ITEM_H
#define LIBCANAL_ARRAY_SINGLE_ITEM_H

#include "Value.h"
#include "ArrayInterface.h"

namespace Canal {
namespace Array {

// The most trivial array type.  It treats all array members as a
// single value.  This means all the operations on the array are
// merged and used to move the single value up in its lattice.
//
// This array type is very imprecise.
class SingleItem : public Value, public Interface
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
    virtual std::string toString() const;

public: // Implementation of Array::Interface.
    // Implementation of Array::Interface::getItem().
    virtual std::vector<Value*> getItem(const Value &offset) const;
    // Implementation of Array::Interface::getItem().
    virtual Value *getItem(uint64_t offset) const;
    // Implementation of Array::Interface::setItem().
    virtual void setItem(const Value &offset, const Value &value);
    // Implementation of Array::Interface::setItem().
    virtual void setItem(uint64_t offset, const Value &value);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_SINGLE_ITEM_H
