#include "lib/FloatInterval.h"
#include "lib/Utils.h"
#include "lib/Environment.h"

using namespace Canal;

static Environment *gEnvironment;

static void
testConstructors()
{
    Float::Interval i(*gEnvironment, llvm::APFloat::IEEEsingle);
    CANAL_ASSERT(i.isBottom());
    CANAL_ASSERT(!i.isTop());
    CANAL_ASSERT(!i.isNaN());
    CANAL_ASSERT(&i.getSemantics() == &llvm::APFloat::IEEEsingle);
}

static void
testJoin()
{
    Float::Interval bot(*gEnvironment, llvm::APFloat::IEEEsingle);
    Float::Interval top(*gEnvironment, llvm::APFloat::IEEEsingle);
    top.setTop();

    Float::Interval test(*gEnvironment, llvm::APFloat::IEEEsingle);
    CANAL_ASSERT(test.join(bot).isBottom());
    CANAL_ASSERT(test.join(top).isTop());
    CANAL_ASSERT(test.join(bot).isTop());
    CANAL_ASSERT(!test.isBottom());

    Float::Interval ten(*gEnvironment,
                        llvm::APFloat(-10.0),
                        llvm::APFloat(10.0));

    test.setBottom();
    CANAL_ASSERT(test.join(ten) == ten);
    CANAL_ASSERT(test.join(ten) == ten);
    CANAL_ASSERT(test.join(bot) == ten);
    CANAL_ASSERT(test.join(top).isTop());
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();
    testJoin();

    delete gEnvironment;
    return 0;
}
