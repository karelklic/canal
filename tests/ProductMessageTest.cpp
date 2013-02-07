#include "lib/ProductMessage.h"
#include "lib/ProductMessageField.h"
#include "lib/Utils.h"

using namespace Canal::Product;

class DummyMessageField: public MessageField {
public:
    char meetCounter;
    char cloneCounter;

    DummyMessageField() : MessageField(MessageField::DummyMessageFieldKind) {}

    virtual DummyMessageField &meet(const MessageField &other) {
        meetCounter++;
        return *this;
    }

    virtual MessageField* clone() const {
        DummyMessageField* cloned = new DummyMessageField(*this);
        cloned->cloneCounter++;
        return cloned;
    }
};

static void
testMeet()
{
    Message message1;
    Message message2;

    // Empty messages
    message1.meet(message2);
    CANAL_ASSERT(message1.mFields.size() == 0);

    // Skipping meet if owner's filed is set but other isn't
    message1.mFields[MessageField::DummyMessageFieldKind] = new DummyMessageField;
    message1.meet(message2);
    CANAL_ASSERT(((DummyMessageField*) message1.mFields[MessageField::DummyMessageFieldKind])->meetCounter == 0);

    // Meet is called if both fields are set
    message2.mFields[MessageField::DummyMessageFieldKind] = new DummyMessageField;
    message1.meet(message2);
    CANAL_ASSERT(((DummyMessageField*) message1.mFields[MessageField::DummyMessageFieldKind])->meetCounter == 1);
    CANAL_ASSERT(((DummyMessageField*) message2.mFields[MessageField::DummyMessageFieldKind])->meetCounter == 0);

    // Clone is called if owner's field is not set, but other is
    message1.mFields[MessageField::DummyMessageFieldKind] = NULL;
    message1.meet(message2);
    CANAL_ASSERT(((DummyMessageField*) message1.mFields[MessageField::DummyMessageFieldKind])->cloneCounter == 1);
}

int
main(int argc, char **argv)
{
    testMeet();

    return 0;
}
