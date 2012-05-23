#ifndef LIBCANAL_ARRAY_H
#define LIBCANAL_ARRAY_H

#include "Value.h"

namespace Canal {
namespace Array {

class Array : public Value
{
public:
    Value *get(Value *offset) = 0;
    void set(Value *offset, Value *value) = 0;
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_H
