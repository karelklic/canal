#include "ArrayInterface.h"
#include "Value.h"

namespace Canal {
namespace Array {

Value *Interface::getValue(const Value &offset) const
{
    Value *result = NULL;
    std::vector<Value*> items(getItem(offset));
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
