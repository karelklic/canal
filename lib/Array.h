#ifndef CANAL_ARRAY_H
#define CANAL_ARRAY_H

#include "Value.h"

namespace Canal {
namespace Array {

// We treat all array members as a single value.  This means, that all
// the operations on the array are merged and used to move the single
// value up in its lattice.
class SingleItem : public Value
{
public:
    Value *mItemValue;

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
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;
};

} // namespace Array
} // namespace Canal

#endif // CANAL_ARRAY_H
