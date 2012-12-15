#include "lib/FloatInterval.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

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

static void
testDivisionByZero() {
    Float::Interval zero(*gEnvironment, llvm::APFloat(0.0f)),
            one(*gEnvironment, llvm::APFloat(1.0f)),
            two(*gEnvironment, llvm::APFloat(2.0f)),
            one_two(one),
            zero_one(zero),
            minusone_zero(*gEnvironment, llvm::APFloat(-1.0f)),
            result(zero);
    one_two.join(two);
    zero_one.join(one);
    minusone_zero.join(zero);

    llvm::APFloat res(0.0f);

    //Fdiv test
    CANAL_ASSERT(result.fdiv(one, zero).isTop());
    CANAL_ASSERT(result.fdiv(zero, zero).isTop());

    result.fdiv(one_two, zero_one);
    res = result.getMin(); CANAL_ASSERT(res.compare(llvm::APFloat(1.0f)) == llvm::APFloat::cmpEqual); //One to infinity
    res = result.getMax(); CANAL_ASSERT(res.isInfinity() && !res.isNegative());

    result.fdiv(one_two, minusone_zero); //Division by -1 to 0
    res = result.getMin(); CANAL_ASSERT(res.isInfinity() && res.isNegative()); //Negative infinity minus one
    res = result.getMax(); CANAL_ASSERT(res.compare(llvm::APFloat(-1.0f)) == llvm::APFloat::cmpEqual); //Unsigned zero to two
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
    testDivisionByZero();

    delete gEnvironment;
    return 0;
}
