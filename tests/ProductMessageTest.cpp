#include "lib/ProductMessage.h"
#include "lib/ProductMessageField.h"
#include "lib/Utils.h"

using namespace Canal;

class FakeMessageField: public Product::MessageField {
public:
    char meetCounter;
    char cloneCounter;

    virtual FakeMessageField &meet(const MessageField &other) {
        meetCounter++;
        return *this;
    }

    virtual MessageField* clone() const {
        FakeMessageField* cloned = new FakeMessageField(*this);
        cloned->cloneCounter++;
        return cloned;
    }
};

static void
testMeet()
{
    Product::Message message1;
    Product::Message message2;

    // Empty messages
    message1.meet(message2);
    CANAL_ASSERT(message1.mFields.size() == 0);

    // Skipping meet if both fields aren't set
    message1.mFields.push_back(NULL);
    message2.mFields.push_back(NULL);
    message1.meet(message2);
    CANAL_ASSERT(!message1.mFields[0]);

    // Skipping meet if owner's filed is set but other isn't
    message1.mFields.push_back(new FakeMessageField);
    message2.mFields.push_back(NULL);
    message1.meet(message2);
    CANAL_ASSERT(((FakeMessageField*) message1.mFields[1])->meetCounter == 0);

    // Meet is called if both fields are set
    message2.mFields.pop_back();
    message2.mFields.push_back(new FakeMessageField);
    message1.meet(message2);
    CANAL_ASSERT(((FakeMessageField*) message1.mFields[1])->meetCounter == 1);
    CANAL_ASSERT(((FakeMessageField*) message2.mFields[1])->meetCounter == 0);

    // Clone is called if owner's field is not set, but other is
    message1.mFields.pop_back();
    message1.mFields.push_back(NULL);
    message1.meet(message2);
    CANAL_ASSERT(((FakeMessageField*) message1.mFields[1])->cloneCounter == 1);
}

int
main(int argc, char **argv)
{
    testMeet();

    return 0;
}
