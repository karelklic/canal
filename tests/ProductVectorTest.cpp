#include "lib/ProductVector.h"
#include "lib/Utils.h"
#include "lib/Interpreter.h"
#include "lib/ProductMessage.h"
#include "lib/ProductMessageField.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

using namespace Canal::Product;

static Canal::Environment* gEnvironment;
static MessageField::MessageFieldKind gKind = MessageField::DummyMessageFieldKind;

class DummyField : public MessageField {
public:
    char num;

    DummyField(char i) : MessageField(gKind), num(i) {}

    virtual MessageField* clone() const { return new DummyField(num); }
    virtual MessageField& meet(const MessageField& other)
    {
        num = ((const DummyField&) other).num;
        return *this;
    }

};


class FakeDomain : public Canal::Domain {
public:
    char num;

    FakeDomain(Canal::Environment &env)
      : Domain(env, Domain::StructureKind),
        num(0) {}


    virtual void extract(Message &message) const
    {
        message.mFields[gKind] = new DummyField(num + 1);
    }

    virtual void refine(const Message &message) {
        if ( message.mFields.find(gKind) != message.mFields.end() )
            num = ((DummyField*) message.mFields.find(gKind)->second)->num;
    }

    virtual Domain* clone() const { return NULL; }
    virtual Domain& join(const Domain& value) { return *this; }
    virtual Domain& meet(const Domain& value) { return *this; }
    virtual size_t memoryUsage() const { return 0; }
    virtual bool operator<(const Domain& value) const { return false; }
    virtual bool operator==(const Domain& value) const { return false; }
    virtual void setZero(const llvm::Value* place) {}
    virtual std::string toString() const { return "fakeDomain"; }
};

static void
testCollaborate()
{
    FakeDomain* a = new FakeDomain(*gEnvironment);
    FakeDomain* b = new FakeDomain(*gEnvironment);
    FakeDomain* c = new FakeDomain(*gEnvironment);

    Vector vector(*gEnvironment);

    // Empty vector
    vector.collaborate();

    // Multiple domains
    vector.mValues.push_back(a);
    vector.mValues.push_back(b);
    vector.mValues.push_back(c);
    vector.collaborate();

    // Domains are iterated in order
    // every refine() increases message.num
    // every extract() sets this.num to message.num
    CANAL_ASSERT(a->num == 0);
    CANAL_ASSERT(b->num == 1);
    CANAL_ASSERT(c->num == 2);
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Canal::Environment(module);

    testCollaborate();

    delete gEnvironment;
    return 0;
}
