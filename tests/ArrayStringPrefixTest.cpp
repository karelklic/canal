#include "lib/ArrayStringPrefix.h"
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
    Array::StringPrefix stringPrefix(*gEnvironment);
    CANAL_ASSERT(stringPrefix.isBottom());

    Array::StringPrefix stringPrefix2(*gEnvironment, "test");
    CANAL_ASSERT(!stringPrefix2.isBottom());
    CANAL_ASSERT(stringPrefix2.mPrefix == "test");
}

static void
testSetZero()
{
    Array::StringPrefix prefix(*gEnvironment);
    prefix.setZero(NULL);
    CANAL_ASSERT(prefix.isTop());
}

static void
testEquality()
{
    Array::StringPrefix bottom(*gEnvironment),
        nonBottom1(*gEnvironment, "test"),
        nonBottom2(*gEnvironment, "aaa"),
        top(*gEnvironment, "");

    CANAL_ASSERT(bottom == bottom);
    CANAL_ASSERT(bottom != nonBottom1);
    CANAL_ASSERT(nonBottom1 != bottom);
    CANAL_ASSERT(nonBottom1 == nonBottom1);
    CANAL_ASSERT(nonBottom1 != nonBottom2);
    CANAL_ASSERT(nonBottom2 != nonBottom1);
    CANAL_ASSERT(nonBottom2 == nonBottom2);
    CANAL_ASSERT(nonBottom2 != top);
    CANAL_ASSERT(top != nonBottom2);
    CANAL_ASSERT(top == top);
    CANAL_ASSERT(bottom != top);
    CANAL_ASSERT(top != bottom);
}

static void
testJoin()
{
    Array::StringPrefix bottom(*gEnvironment),
        withCommonPrefix1(*gEnvironment, "testone"),
        withCommonPrefix2(*gEnvironment, "testtwothree"),
        withoutCommonPrefix(*gEnvironment, "notest"),
        top(*gEnvironment, "");

    // bottom vs bottom
    Array::StringPrefix result1 = bottom.join(bottom);
    CANAL_ASSERT(result1.isBottom());

    // non-bottom vs bottom
    Array::StringPrefix result2 = withCommonPrefix1.join(bottom);
    CANAL_ASSERT(!result2.isBottom());
    CANAL_ASSERT(result2.mPrefix == "testone");
    CANAL_ASSERT(!result2.isTop());

    // bottom vs non-bottom
    Array::StringPrefix result3 = bottom.join(withCommonPrefix1);
    CANAL_ASSERT(!result3.isBottom());
    CANAL_ASSERT(result3.mPrefix == "testone");
    CANAL_ASSERT(!result3.isTop());

    // non-bottom vs non-bottom with common prefix
    Array::StringPrefix result4 = withCommonPrefix1.join(withCommonPrefix2);
    CANAL_ASSERT(!result4.isBottom());
    CANAL_ASSERT(result4.mPrefix == "test");
    CANAL_ASSERT(!result4.isTop());

    // non-bottom vs non-bottom with common prefix switcheroo
    Array::StringPrefix result5 = withCommonPrefix2.join(withCommonPrefix1);
    CANAL_ASSERT(!result5.isBottom());
    CANAL_ASSERT(result5.mPrefix == "test");
    CANAL_ASSERT(!result5.isTop());

    // non-bottom vs non-bottom without common prefix
    Array::StringPrefix result6 = withCommonPrefix2.join(withoutCommonPrefix);
    CANAL_ASSERT(result6.isTop());

    // non-bottom vs non-bottom without common prefix switcheroo
    Array::StringPrefix result7 = withoutCommonPrefix.join(withCommonPrefix2);
    CANAL_ASSERT(result7.isTop());

    // non-bottom vs top
    Array::StringPrefix result8 = withoutCommonPrefix.join(top);
    CANAL_ASSERT(result8.isTop());

    // top vs non-bottom
    Array::StringPrefix result9 = top.join(withoutCommonPrefix);
    CANAL_ASSERT(result9.isTop());

    // top vs top
    Array::StringPrefix result10 = top.join(top);
    CANAL_ASSERT(result10.isTop());

    // bottom vs top
    Array::StringPrefix result11 = bottom.join(top);
    CANAL_ASSERT(result11.isTop());

    // top vs bottom
    Array::StringPrefix result12 = top.join(bottom);
    CANAL_ASSERT(result12.isTop());
}

static void
testMeet()
{
   //TODO 
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y; // Call llvm_shutdown() on exit?

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();
    testSetZero();
    testEquality();
    testJoin();
    testMeet();

    delete gEnvironment;
    return 0;
}
