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
    const llvm::Type &elementType = *llvm::Type::getInt8Ty(
        gEnvironment->getContext());

    const llvm::ArrayType &type = *llvm::ArrayType::get(&elementType, 10);

    Array::StringPrefix stringPrefix(*gEnvironment, type);
    CANAL_ASSERT(stringPrefix.isBottom());

    Array::StringPrefix stringPrefix2(*gEnvironment, "test");
    CANAL_ASSERT(!stringPrefix2.isBottom());
    CANAL_ASSERT(stringPrefix2.mPrefix == "test");
}

static void
testSetZero()
{
    const llvm::Type &elementType = *llvm::Type::getInt8Ty(
        gEnvironment->getContext());

    const llvm::ArrayType &type = *llvm::ArrayType::get(&elementType, 10);

    Array::StringPrefix prefix(*gEnvironment, type);
    prefix.setZero(NULL);
    CANAL_ASSERT(prefix.isTop());
}

static void
testEquality()
{
    const llvm::Type &elementType = *llvm::Type::getInt8Ty(
        gEnvironment->getContext());

    const llvm::ArrayType &type = *llvm::ArrayType::get(&elementType, 10);

    Array::StringPrefix prefix1(*gEnvironment, type),
        prefix2(*gEnvironment, type),
        prefix3(*gEnvironment, "test"),
        prefix4(*gEnvironment, "test"),
        prefix5(*gEnvironment, "aaa"),
        prefix6(*gEnvironment, "");

    CANAL_ASSERT(prefix1 == prefix1);
    CANAL_ASSERT(prefix1 == prefix2);
    CANAL_ASSERT(prefix1 != prefix3);
    CANAL_ASSERT(prefix3 == prefix4);
    CANAL_ASSERT(prefix3 != prefix5);
    CANAL_ASSERT(prefix1 != prefix6);
    CANAL_ASSERT(prefix3 != prefix6);
    CANAL_ASSERT(prefix6 == prefix6);
}

static void
testJoin()
{
    const llvm::Type &elementType = *llvm::Type::getInt8Ty(
        gEnvironment->getContext());

    const llvm::ArrayType &type = *llvm::ArrayType::get(&elementType, 10);

    // bottom vs bottom
    Array::StringPrefix bottom1(*gEnvironment, type);
    Array::StringPrefix result1 = bottom1.join(bottom1);
    CANAL_ASSERT(result1.isBottom());

    // non-bottom vs bottom
    Array::StringPrefix withPrefix2(*gEnvironment, "testone"),
        bottom2(*gEnvironment, type);
    Array::StringPrefix result2 = withPrefix2.join(bottom2);
    CANAL_ASSERT(!result2.isBottom());
    CANAL_ASSERT(result2.mPrefix == "testone");
    CANAL_ASSERT(!result2.isTop());

    // bottom vs non-bottom
    Array::StringPrefix bottom3(*gEnvironment, type),
        withPrefix3(*gEnvironment, "testone");
    Array::StringPrefix result3 = bottom3.join(withPrefix3);
    CANAL_ASSERT(!result3.isBottom());
    CANAL_ASSERT(result3.mPrefix == "testone");
    CANAL_ASSERT(!result3.isTop());

    // non-bottom vs non-bottom with common prefix
    Array::StringPrefix withPrefix4a(*gEnvironment, "testone"),
        withPrefix4b(*gEnvironment, "testtwothree");
    Array::StringPrefix result4 = withPrefix4a.join(withPrefix4b);
    CANAL_ASSERT(!result4.isBottom());
    CANAL_ASSERT(result4.mPrefix == "test");
    CANAL_ASSERT(!result4.isTop());

    // non-bottom vs non-bottom with common prefix switcheroo
    Array::StringPrefix withPrefix5a(*gEnvironment, "testtwothree"),
        withPrefix5b(*gEnvironment, "testone");
    Array::StringPrefix result5 = withPrefix5a.join(withPrefix5b);
    CANAL_ASSERT(!result5.isBottom());
    CANAL_ASSERT(result5.mPrefix == "test");
    CANAL_ASSERT(!result5.isTop());

    // non-bottom vs non-bottom without common prefix
    Array::StringPrefix withPrefix6(*gEnvironment, "testone"),
        withoutPrefix6(*gEnvironment, "xyz");
    Array::StringPrefix result6 = withPrefix6.join(withoutPrefix6);
    CANAL_ASSERT(result6.isTop());

    // non-bottom vs non-bottom without common prefix switcheroo
    Array::StringPrefix withoutPrefix7(*gEnvironment, "xyz"),
        withPrefix7(*gEnvironment, "testone");
    Array::StringPrefix result7 = withoutPrefix7.join(withPrefix7);
    CANAL_ASSERT(result7.isTop());

    // non-bottom vs top
    Array::StringPrefix withoutPrefix8(*gEnvironment, "xyz"),
        top8(*gEnvironment, "");
    Array::StringPrefix result8 = withoutPrefix8.join(top8);
    CANAL_ASSERT(result8.isTop());

    // top vs non-bottom
    Array::StringPrefix top9(*gEnvironment, ""),
        withoutPrefix9(*gEnvironment, "xyz");
    Array::StringPrefix result9 = top9.join(withoutPrefix9);
    CANAL_ASSERT(result9.isTop());

    // top vs top
    Array::StringPrefix top10(*gEnvironment, "");
    Array::StringPrefix result10 = top10.join(top10);
    CANAL_ASSERT(result10.isTop());

    // bottom vs top
    Array::StringPrefix bottom11(*gEnvironment, type),
        top11(*gEnvironment, "");
    Array::StringPrefix result11 = bottom11.join(top11);
    CANAL_ASSERT(result11.isTop());

    // top vs bottom
    Array::StringPrefix top12(*gEnvironment, ""),
        bottom12(*gEnvironment, type);
    Array::StringPrefix result12 = top12.join(bottom12);
    CANAL_ASSERT(result12.isTop());
}

static void
testMeet()
{
    const llvm::Type &elementType = *llvm::Type::getInt8Ty(
        gEnvironment->getContext());

    const llvm::ArrayType &type = *llvm::ArrayType::get(&elementType, 10);

    // bottom vs bottom
    Array::StringPrefix bottom1(*gEnvironment, type);
    Array::StringPrefix result1 = bottom1.meet(bottom1);
    CANAL_ASSERT(result1.isBottom());

    // bottom vs prefix
    Array::StringPrefix bottom2(*gEnvironment, type),
        prefix2(*gEnvironment, "abc");
    Array::StringPrefix result2 = bottom2.meet(prefix2);
    CANAL_ASSERT(result2.isBottom());

    // prefix vs bottom
    Array::StringPrefix prefix3(*gEnvironment, "abc"),
        bottom3(*gEnvironment, type);
    Array::StringPrefix result3 = prefix3.meet(bottom3);
    CANAL_ASSERT(result3.isBottom());

    // prefix vs with prefix
    Array::StringPrefix prefix4(*gEnvironment, "abc"),
        withPrefix4(*gEnvironment, "abcdef");

    Array::StringPrefix result4 = prefix4.meet(withPrefix4);
    CANAL_ASSERT(!result4.isBottom());
    CANAL_ASSERT(result4.mPrefix == "abcdef");
    CANAL_ASSERT(!result4.isTop());

    // with prefix vs prefix
    Array::StringPrefix withPrefix5(*gEnvironment, "abcghi"),
        prefix5(*gEnvironment, "abc");
    Array::StringPrefix result5 = withPrefix5.meet(prefix5);
    CANAL_ASSERT(!result5.isBottom());
    CANAL_ASSERT(result5.mPrefix == "abcghi");
    CANAL_ASSERT(!result5.isTop());

    // with prefix vs without prefix
    Array::StringPrefix withPrefix6(*gEnvironment, "abcjkl"),
        withoutPrefix6(*gEnvironment, "xyz");
    Array::StringPrefix result6 = withPrefix6.meet(withoutPrefix6);
    CANAL_ASSERT(result6.isBottom());

    // without prefix vs with prefix
    Array::StringPrefix withoutPrefix7(*gEnvironment, "uvw"),
        withPrefix7(*gEnvironment, "abcmno");
    Array::StringPrefix result7 = withoutPrefix7.meet(withPrefix7);
    CANAL_ASSERT(result7.isBottom());

    // prefix vs top
    Array::StringPrefix prefix8(*gEnvironment, "abc"),
        top8(*gEnvironment, "");
    Array::StringPrefix result8 = prefix8.meet(top8);
    CANAL_ASSERT(!result8.isBottom());
    CANAL_ASSERT(result8.mPrefix == "abc");
    CANAL_ASSERT(!result8.isTop());

    // top vs prefix
    Array::StringPrefix top9(*gEnvironment, ""),
        prefix9(*gEnvironment, "abc");
    Array::StringPrefix result9 = top9.meet(prefix9);
    CANAL_ASSERT(!result9.isBottom());
    CANAL_ASSERT(result9.mPrefix == "abc");
    CANAL_ASSERT(!result9.isTop());

    // top vs top
    Array::StringPrefix top10(*gEnvironment, "");
    Array::StringPrefix result10 = top10.meet(top10);
    CANAL_ASSERT(result10.isTop());

    // top vs bottom
    Array::StringPrefix top11(*gEnvironment, ""),
        bottom11(*gEnvironment, type);
    Array::StringPrefix result11 = top11.meet(bottom11);
    CANAL_ASSERT(result11.isBottom());

    // bottom vs top
    Array::StringPrefix bottom12(*gEnvironment, type),
        top12(*gEnvironment, "");
    Array::StringPrefix result12 = bottom12.meet(top12);
    CANAL_ASSERT(result12.isBottom());
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
