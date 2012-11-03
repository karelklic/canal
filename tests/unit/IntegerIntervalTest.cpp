#include "lib/IntegerInterval.h"
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
    Integer::Interval interval(*gEnvironment, 1);
    CANAL_ASSERT(interval.mEmpty && !interval.mSignedTop && !interval.mUnsignedTop);
    CANAL_ASSERT(interval.mSignedFrom == interval.mSignedTo);
    CANAL_ASSERT(interval.mUnsignedFrom == interval.mUnsignedTo);
    CANAL_ASSERT(interval.mSignedFrom.getSExtValue() == 0);
    CANAL_ASSERT(interval.mUnsignedFrom.getZExtValue() == 0);
}

static void
testSignedMin()
{
}

static void
testSignedMax()
{
}

static void
testUnsignedMin()
{
}

static void
testUnsignedMax()
{
}

static void
testIsSingleValue()
{
}

static void
testCloneCleaned()
{
}

static void
testEquality()
{
    Integer::Interval interval1(*gEnvironment, 1),
        interval2(*gEnvironment, 1),
        interval3(*gEnvironment, 2);

    // Test empty intervals.
    CANAL_ASSERT(interval1 == interval2);
    CANAL_ASSERT(interval1 != interval3);
}

static void
testTrunc()
{
    // Test truncation from i8 00001010 to i3.
    Integer::Interval interval1(*gEnvironment, 3);
    interval1.trunc(Integer::Interval(*gEnvironment, llvm::APInt(8, 10)));
    CANAL_ASSERT(interval1.isTop());
    CANAL_ASSERT(interval1.getBitWidth() == 3);

    // Test truncation from i8 00000100 to i3.
    Integer::Interval interval2(*gEnvironment, 3);
    interval2.trunc(Integer::Interval(*gEnvironment, llvm::APInt(8, 4)));
    llvm::APInt res;
    CANAL_ASSERT(interval2.isSingleValue() && interval2.unsignedMin(res) && res == 4);
    CANAL_ASSERT(interval2.getBitWidth() == 3);

    // Test truncation from i32 00000001 to i1.
    Integer::Interval interval3(*gEnvironment, 1);
    interval3.trunc(Integer::Interval(*gEnvironment, llvm::APInt(32, 1)));
    CANAL_ASSERT(interval3.isSingleValue() && interval3.unsignedMin(res) && res == 1);
    CANAL_ASSERT(interval3.getBitWidth() == 1);
}

static void
testMerge()
{
}

static void
testMemoryUsage()
{
}

static void
testToString()
{
}

static void
testMatchesString()
{
}

static void
testAdd()
{
}

static void
testSub()
{
}

static void
testMul()
{
}

static void
testUDiv()
{
}

static void
testSDiv()
{
}

static void
testURem()
{
}

static void
testSRem()
{
}

static void
testShl()
{
}

static void
testLshr()
{
}

static void
testAshr()
{
}

static void
testAnd()
{
}

static void
testOr()
{
}

static void
testXor()
{
}

static void
testIcmp()
{
}

static void
testAccuracy()
{
}

static void
testIsBottom()
{
}

static void
testSetBottom()
{
}

static void
testIsTop()
{
}

static void
testSetTop()
{
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();
    testSignedMin();
    testSignedMax();
    testUnsignedMin();
    testUnsignedMax();
    testIsSingleValue();
    testCloneCleaned();
    testEquality();
    testMerge();
    testMemoryUsage();
    testToString();
    testMatchesString();
    testAdd();
    testSub();
    testMul();
    testUDiv();
    testSDiv();
    testURem();
    testSRem();
    testShl();
    testLshr();
    testAshr();
    testAnd();
    testOr();
    testXor();
    testIcmp();
    testAccuracy();
    testIsBottom();
    testSetBottom();
    testIsTop();
    testSetTop();
    testTrunc();

    delete gEnvironment;
    return 0;
}
