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
    for(const_iterator it = other.mFields.begin(); it != other.mFields.end(); ++it) {
      if (mFields[it->first])
        mFields[it->first]->meet(*it->second);
      else
        mFields[it->first] = it->second->clone();
    }

    return *this;
}


} // namespace Canal
} // namespace Product
