#ifndef LIBCANAL_ARRAY_EXACT_SIZE_H
#define LIBCANAL_ARRAY_EXACT_SIZE_H

#include "Value.h"
#include "ArrayInterface.h"

namespace Canal {
namespace Array {

// Array with exact size and limited length.  It keeps all array
// members separately, not losing precision at all.
class ExactSize : public Value, public Interface
{
public:
    std::vector<Value*> mValues;

public:
    ExactSize();
    ExactSize(const ExactSize &exactSize);
    virtual ~ExactSize();

    size_t size() const { return mValues.size(); }

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual ExactSize *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;

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

#endif // LIBCANAL_ARRAY_EXACT_SIZE_H
