#include "ProductMessage.h"
#include "ProductMessageField.h"
#include "Utils.h"

namespace Canal {
namespace Product {

Message::~Message()
{
    for(iterator it = mFields.begin(); it != mFields.end(); ++it)
        delete it->second;
}

Message& Message::meet(const Message& other)
{
    Map otherFields = other.mFields;

    for(iterator it = otherFields.begin(); it != otherFields.end(); ++it) {
      if (mFields[it->first])
        mFields[it->first]->meet(*it->second);
      else
        mFields[it->first] = it->second->clone();
    }
    
    return *this;
}


} // namespace Canal
} // namespace Product
