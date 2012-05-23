#ifndef LIBCANAL_ARRAY_EXACT_LIMITED_SIZE_H
#define LIBCANAL_ARRAY_EXACT_LIMITED_SIZE_H

#include "Array.h"

namespace Canal {
namespace Array {

// Array with exact size and limited length.  It keeps all array
// members separately, not losing precision at all.
class ExactLimitedSize : public Array
{
public:
    std::vector<Value*> mValues;
    size_t mSize;

public:
    ExactLimitedSize();
    virtual ~ExactLimitedSize();

    // Implementation of Value::clone().
    // Covariant return type.
    virtual ExactLimitedSize *clone() const;
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

#endif // LIBCANAL_ARRAY_EXACT_LIMITED_SIZE_H
