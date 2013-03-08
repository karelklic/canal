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

    Float::Interval zero(*gEnvironment, llvm::APFloat(0.0f)),
            allInfinity(*gEnvironment, llvm::APFloat::getInf(zero.getSemantics(), true));
    allInfinity.join(Float::Interval(*gEnvironment, llvm::APFloat::getInf(zero.getSemantics(), false)));
    CANAL_ASSERT(allInfinity != zero);
    CANAL_ASSERT(zero.join(allInfinity) == allInfinity);
}

static void
testComparison()
{
    Float::Interval zero(*gEnvironment, llvm::APFloat(0.0f)),
            bot(*gEnvironment, llvm::APFloat::IEEEsingle),
            top(*gEnvironment, llvm::APFloat::IEEEsingle);
    top.setTop();

    CANAL_ASSERT(top == top);
    CANAL_ASSERT(bot == bot);
    CANAL_ASSERT(zero == zero);

    CANAL_ASSERT(top != zero);
    CANAL_ASSERT(zero != top);

    CANAL_ASSERT(top != bot);
    CANAL_ASSERT(bot != top);

    CANAL_ASSERT(bot != zero);
    CANAL_ASSERT(zero != bot);
}

static void
testDivisionByZero()
{
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

    //Fdiv test
    CANAL_ASSERT(result.fdiv(one, zero).isTop());
    CANAL_ASSERT(result.fdiv(zero, zero).isTop());

    result.fdiv(one_two, zero_one);
    llvm::APFloat min(0.0f), max(0.0f);
    CANAL_ASSERT(result.getMinMax(min, max));
    CANAL_ASSERT(min.compare(llvm::APFloat(1.0f)) == llvm::APFloat::cmpEqual); //One to infinity
    CANAL_ASSERT(max.isInfinity() && !max.isNegative());

    result.fdiv(one_two, minusone_zero); //Division by -1 to 0
    CANAL_ASSERT(result.getMinMax(min, max));
    CANAL_ASSERT(min.isInfinity() && min.isNegative()); //Negative infinity minus one
    CANAL_ASSERT(max.compare(llvm::APFloat(-1.0f)) == llvm::APFloat::cmpEqual); //Unsigned zero to two
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();
    testComparison();
    testJoin();
    testDivisionByZero();

    delete gEnvironment;
    return 0;
}
