#include "lib/ArrayStringSuffix.h"
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
    const llvm::ArrayType &type = *llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);

    Array::StringSuffix suffix1(*gEnvironment, type);
    CANAL_ASSERT(suffix1.isBottom());

    Array::StringSuffix suffix2(*gEnvironment, "test");
    CANAL_ASSERT(!suffix2.isBottom());
    CANAL_ASSERT(suffix2.mSuffix == "test");
}

static void
testSetZero()
{
    const llvm::ArrayType &type = *llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);

    Array::StringSuffix suffix(*gEnvironment, type);
    suffix.setZero(NULL);
    CANAL_ASSERT(suffix.isTop());
}

static void
testEquality()
{
    const llvm::ArrayType &type = *llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);

    Array::StringSuffix suffix1(*gEnvironment, type),
        suffix2(*gEnvironment, type),
        suffix3(*gEnvironment, "test"),
        suffix4(*gEnvironment, "test"),
        suffix5(*gEnvironment, "aaa"),
        suffix6(*gEnvironment, "");

    CANAL_ASSERT(suffix1 == suffix1);
    CANAL_ASSERT(suffix1 == suffix2);
    CANAL_ASSERT(suffix1 != suffix3);
    CANAL_ASSERT(suffix3 == suffix4);
    CANAL_ASSERT(suffix3 != suffix5);
    CANAL_ASSERT(suffix1 != suffix6);
    CANAL_ASSERT(suffix3 != suffix6);
    CANAL_ASSERT(suffix6 == suffix6);
}

static void
testLessThanOperator()
{
    const llvm::ArrayType &type = *llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);

    Array::StringSuffix bottom(*gEnvironment, type),
        suffix1(*gEnvironment, "test"),
        suffix2(*gEnvironment, "est"),
        suffix3(*gEnvironment, "aaa"),
        top(*gEnvironment, "");

    // top vs top
    CANAL_ASSERT((top < top) == false);

    // top vs suffix
    CANAL_ASSERT((top < suffix1) == false);

    // top vs bottom
    CANAL_ASSERT((top < bottom) == false);

    // bottom vs top
    CANAL_ASSERT(bottom < top);

    // bottom vs suffix
    CANAL_ASSERT(bottom < suffix1);

    // bottom vs bottom
    CANAL_ASSERT((bottom < bottom) == false);

    // suffix vs top
    CANAL_ASSERT(suffix1 < top);

    // suffix vs bottom
    CANAL_ASSERT((suffix1 < bottom) == false);

    // suffix vs suffix
    CANAL_ASSERT(suffix1 < suffix2);
    CANAL_ASSERT((suffix2 < suffix1) == false);
    CANAL_ASSERT((suffix2 < suffix3) == false);
}

static void
testJoin()
{
    const llvm::ArrayType &type = *llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);

    // bottom vs bottom
    Array::StringSuffix bottom(*gEnvironment, type);
    bottom.join(bottom);
    CANAL_ASSERT(bottom.isBottom());

    // suffix vs bottom
    Array::StringSuffix suffix1(*gEnvironment, "something");
    suffix1.join(bottom);
    CANAL_ASSERT(!suffix1.isBottom());
    CANAL_ASSERT(suffix1.mSuffix == "something");
    CANAL_ASSERT(!suffix1.isTop());

    // bottom vs suffix
    Array::StringSuffix bottom1(*gEnvironment, type),
        suffix2(*gEnvironment, "else");
    bottom1.join(suffix2);
    CANAL_ASSERT(!bottom1.isTop());
    CANAL_ASSERT(bottom1.mSuffix == "else");
    CANAL_ASSERT(!bottom1.isBottom());

    // bottom vs top
    Array::StringSuffix bottom2(*gEnvironment, type),
        top1(*gEnvironment, "");
    bottom2.join(top1);
    CANAL_ASSERT(bottom2.isTop());

    // top vs bottom
    Array::StringSuffix top2(*gEnvironment, ""),
        bottom3(*gEnvironment, type);
    top2.join(bottom);
    CANAL_ASSERT(top2.isTop());

    // top vs suffix
    Array::StringSuffix top3(*gEnvironment, ""),
        suffix3(*gEnvironment, "test");
    top3.join(suffix3);
    CANAL_ASSERT(top3.isTop());

    // suffix vs top
    Array::StringSuffix suffix4(*gEnvironment, "pile"),
        top4(*gEnvironment, "");
    suffix4.join(top4);
    CANAL_ASSERT(suffix4.isTop());

    // top vs top
    Array::StringSuffix top5(*gEnvironment, "");
    top5.join(top5);
    CANAL_ASSERT(top5.isTop());

    // suffix vs suffix
    Array::StringSuffix suffix5_1(*gEnvironment, "sdfgsun"),
        suffix5_2(*gEnvironment, "fun");
    suffix5_1.join(suffix5_2);
    CANAL_ASSERT(!suffix5_1.isTop());
    CANAL_ASSERT(suffix5_1.mSuffix == "un");
    CANAL_ASSERT(!suffix5_1.isBottom());

    Array::StringSuffix suffix5_3(*gEnvironment, "abcd"),
        suffix5_4(*gEnvironment, "ef");
    suffix5_3.join(suffix5_4);
    CANAL_ASSERT(suffix5_3.isTop());
}

static void
testMeet()
{
    const llvm::ArrayType &type = *llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);

    // bottom vs bottom
    Array::StringSuffix bottom1(*gEnvironment, type);
    bottom1.meet(bottom1);
    CANAL_ASSERT(bottom1.isBottom());

    // bottom vs suffix
    Array::StringSuffix bottom2(*gEnvironment, type),
        suffix1(*gEnvironment, "test");
    bottom2.meet(suffix1);
    CANAL_ASSERT(bottom2.isBottom());

    // suffix vs bottom
    Array::StringSuffix suffix2(*gEnvironment, "abc"),
        bottom3(*gEnvironment, type);
    suffix2.meet(bottom3);
    CANAL_ASSERT(suffix2.isBottom());

    // bottom vs top
    Array::StringSuffix bottom4(*gEnvironment, type),
        top1(*gEnvironment, "");
    bottom4.meet(top1);
    CANAL_ASSERT(bottom4.isBottom());

    // top vs top
    Array::StringSuffix top2(*gEnvironment, "");
    top2.meet(top2);
    CANAL_ASSERT(top2.isTop());

    // top vs suffix
    Array::StringSuffix top3(*gEnvironment, ""),
        suffix3(*gEnvironment, "def");
    top3.meet(suffix3);
    CANAL_ASSERT(!top3.isBottom());
    CANAL_ASSERT(!top3.isTop());
    CANAL_ASSERT(top3.mSuffix == "def");

    // suffix vs top
    Array::StringSuffix suffix4(*gEnvironment, "more"),
        top4(*gEnvironment, "");
    suffix4.meet(top4);
    CANAL_ASSERT(!suffix4.isBottom());
    CANAL_ASSERT(!suffix4.isTop());
    CANAL_ASSERT(suffix4.mSuffix == "more");

    // top vs bottom
    Array::StringSuffix top5(*gEnvironment, ""),
        bottom5(*gEnvironment, type);
    top5.meet(bottom5);
    CANAL_ASSERT(top5.isBottom());

    // suffix vs suffix
    Array::StringSuffix suffix5_1(*gEnvironment, "best"),
        suffix5_2(*gEnvironment, "sometest");
    suffix5_1.meet(suffix5_2);
    CANAL_ASSERT(suffix5_1.isBottom());

    Array::StringSuffix suffix5_3(*gEnvironment, "car"),
        suffix5_4(*gEnvironment, "house");
    suffix5_3.meet(suffix5_4);
    CANAL_ASSERT(suffix5_3.isBottom());
}

int
main (int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();
    testSetZero();
    testEquality();
    testLessThanOperator();
    testJoin();
    testMeet();

    delete gEnvironment;
    return 0;
}

