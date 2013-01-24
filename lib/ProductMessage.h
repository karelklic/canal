#ifndef LIBCANAL_PRODUCT_MESSAGE_H
#define LIBCANAL_PRODUCT_MESSAGE_H

#include <vector>

namespace Canal {
namespace Product {

class MessageField;

class Message
{
public:
    /// Descendat has to fill mFields w/ all its' MessageFields
    std::vector<MessageField*> mFields;

public:
    virtual ~Message();

    Message &meet(const Message &other);
};

} // namespace Product
} // namespace Canal

#endif // LIBCANAL_PRODUCT_MESSAGE_H
