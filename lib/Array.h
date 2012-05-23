#ifndef LIBCANAL_ARRAY_H
#define LIBCANAL_ARRAY_H

namespace Canal {

class Value;

namespace Array {

class Array
{
public:
    virtual ~Array() {};

    virtual Value *get(Value *offset) const = 0;
    virtual void set(Value *offset, Value *value) = 0;
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_H
