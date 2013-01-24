#include "ProductMessage.h"
#include "ProductMessageField.h"
#include "Utils.h"

namespace Canal {
namespace Product {

Message::~Message()
{
    for(size_t i = 0; i != mFields.size(); i++) {
      if (mFields[i])
        delete mFields[i];
    }
}

Message& Message::meet(const Message& other)
{
    std::vector<MessageField*> otherFields = other.mFields;
    CANAL_ASSERT(mFields.size() == otherFields.size());

    for(size_t i = 0; i != mFields.size(); i++) {
      if (mFields[i] && otherFields[i])
        mFields[i]->meet(*otherFields[i]);
      else if (otherFields[i])
        mFields[i] = otherFields[i]->clone();
    }
    
    return *this;
}


} // namespace Canal
} // namespace Product
