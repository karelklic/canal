#ifndef LIBCANAL_ARRAY_H
#define LIBCANAL_ARRAY_H

namespace Canal {

class Value;

namespace Array {

class Array
{
public:
    virtual ~Array() {};

    // Gets the value representing the array item(s) represented by
    // the provided offset.  Caller is responsible for deleting the
    // returned value.
    virtual Value *get(const Value *offset) const = 0;
    // @param value
    //  The method does not take the ownership of this memory.
    virtual void set(const Value *offset, const Value *value) = 0;
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_H
