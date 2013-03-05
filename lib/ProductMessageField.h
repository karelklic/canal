#ifndef LIBCANAL_PRODUCT_MESSAGE_FIELD_H
#define LIBCANAL_PRODUCT_MESSAGE_FIELD_H

namespace Canal {
namespace Product {

class MessageField
{
public:
    /// Discriminator for LLVM-style RTTI (dyn_cast<> et al.)
    enum MessageFieldKind {
        DummyMessageFieldKind,
        FieldMinMaxKind
    };

    const MessageFieldKind mKind;

    MessageFieldKind getKind() const
    {
        return mKind;
    }

public:
    MessageField(MessageFieldKind kind) : mKind(kind) {}
    virtual ~MessageField() {}

    virtual MessageField &meet(const MessageField &other) = 0;
    virtual MessageField* clone() const = 0;
};


} // namespace Product
} // namespace Canal

#endif // LIBCANAL_PRODUCT_MESSAGE_FIELD_H
