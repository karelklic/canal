#ifndef LIBCANAL_ARRAY_H
#define LIBCANAL_ARRAY_H

#include <vector>

namespace Canal {

class Value;

namespace Array {

class Array
{
public:
    virtual ~Array() {};

    // Gets the value representing the array item(s) pointed by
    // the provided offset.  Caller is responsible for deleting the
    // returned value.
    Value *getValue(const Value &offset) const;

    // Get the array items pointed by the provided offset.  Returns
    // internal array items owned by the array.  Caller must not
    // delete the items.
    virtual std::vector<Value*> getItems(const Value &offset) const = 0;

    // @param value
    //  The method does not take the ownership of this memory.
    virtual void set(const Value &offset, const Value &value) = 0;
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_H
