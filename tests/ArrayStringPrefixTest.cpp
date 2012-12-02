#include "lib/ArrayStringPrefix.h"
#include "lib/Utils.h"
#include "lib/Environment.h"

using namespace Canal;

static Environment *gEnvironment;

static void
testConstructors()
{
    llvm::Type &elementType = *llvm::Type::getInt8Ty(
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
    llvm::Type &elementType = *llvm::Type::getInt8Ty(
        gEnvironment->getContext());

    const llvm::ArrayType &type = *llvm::ArrayType::get(&elementType, 10);

    Array::StringPrefix prefix(*gEnvironment, type);
    prefix.setZero(NULL);
    CANAL_ASSERT(prefix.isTop());
}

static void
testEquality()
{
    llvm::Type &elementType = *llvm::Type::getInt8Ty(
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
    llvm::Type &elementType = *llvm::Type::getInt8Ty(
        gEnvironment->getContext());

    const llvm::ArrayType &type = *llvm::ArrayType::get(&elementType, 10);

    Array::StringPrefix prefix1(*gEnvironment, type),
        prefix2(*gEnvironment, "testone"),
        prefix3(*gEnvironment, "testtwothree"),
        prefix4(*gEnvironment, "notest"),
        prefix5(*gEnvironment, "");

    // bottom vs bottom
    Array::StringPrefix prefix6 = prefix1.join(prefix1);
    CANAL_ASSERT(prefix6.isBottom());

    // non-bottom vs bottom
    Array::StringPrefix prefix7 = prefix2.join(prefix1);
    CANAL_ASSERT(!prefix7.isBottom());
    CANAL_ASSERT(prefix7.mPrefix == "testone");
    CANAL_ASSERT(!prefix7.isTop());

    // bottom vs non-bottom
    Array::StringPrefix prefix8 = prefix1.join(prefix2);
    CANAL_ASSERT(!prefix8.isBottom());
    CANAL_ASSERT(prefix8.mPrefix == "testone");
    CANAL_ASSERT(!prefix8.isTop());

    // non-bottom vs non-bottom with common prefix
    Array::StringPrefix prefix9 = prefix2.join(prefix3);
    CANAL_ASSERT(!prefix9.isBottom());
    CANAL_ASSERT(prefix9.mPrefix == "test");
    CANAL_ASSERT(!prefix9.isTop());

    // non-bottom vs non-bottom with common prefix switcheroo
    Array::StringPrefix prefix10 = prefix3.join(prefix2);
    CANAL_ASSERT(!prefix10.isBottom());
    CANAL_ASSERT(prefix10.mPrefix == "test");
    CANAL_ASSERT(!prefix10.isTop());

    // non-bottom vs non-bottom without common prefix
    Array::StringPrefix prefix11 = prefix3.join(prefix4);
    CANAL_ASSERT(prefix11.isTop());

    // non-bottom vs non-bottom without common prefix switcheroo
    Array::StringPrefix prefix12 = prefix4.join(prefix3);
    CANAL_ASSERT(prefix12.isTop());

    // non-bottom vs top
    Array::StringPrefix prefix13 = prefix4.join(prefix5);
    CANAL_ASSERT(prefix13.isTop());

    // top vs non-bottom
    Array::StringPrefix prefix14 = prefix5.join(prefix4);
    CANAL_ASSERT(prefix14.isTop());

    // top vs top
    Array::StringPrefix prefix15 = prefix5.join(prefix5);
    CANAL_ASSERT(prefix15.isTop());

    // bottom vs top
    Array::StringPrefix prefix16 = prefix1.join(prefix5);
    CANAL_ASSERT(prefix16.isTop());

    // top vs bottom
    Array::StringPrefix prefix17 = prefix5.join(prefix1);
    CANAL_ASSERT(prefix17.isTop());
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
