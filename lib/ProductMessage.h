#ifndef LIBCANAL_PRODUCT_MESSAGE_H
#define LIBCANAL_PRODUCT_MESSAGE_H

#include "ProductMessageField.h"
#include <map>

namespace Canal {
namespace Product {

class Message
{
public:
    typedef std::map<MessageField::MessageFieldKind, MessageField*> Map;
    typedef Map::iterator iterator;
    typedef Map::const_iterator const_iterator;

    Map mFields;

public:
    virtual ~Message();

    Message &meet(const Message &other);
};

} // namespace Product
} // namespace Canal

#endif // LIBCANAL_PRODUCT_MESSAGE_H
