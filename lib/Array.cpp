#include "Array.h"
#include "Value.h"

namespace Canal {
namespace Array {

Value *Array::getValue(const Value &offset) const
{
    Value *result = NULL;
    std::vector<Value*> items(getItems(offset));
    std::vector<Value*>::const_iterator it = items.begin();
    for (; it != items.end(); ++it)
    {
        if (!result)
            result = (*it)->clone();
        else
            result->merge(**it);
    }

    return result;
}

} // namespace Array
} // namespace Canal
