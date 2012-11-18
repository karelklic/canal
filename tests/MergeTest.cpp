#include "lib/IntegerInterval.h"
#include "lib/IntegerEnumeration.h"
#include "lib/IntegerBitfield.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>
#include <vector>
#include <iostream>

using namespace Canal;

static llvm::APInt
APIntFactory(int number)
{
    return llvm::APInt(sizeof(int)*8, number, number < 0);
}

static Integer::Interval
IntervalFactory(const Environment &environment, int number)
{
    return Integer::Interval(environment, APIntFactory(number));
}

static Integer::Bitfield
BitfieldFactory(const Environment &environment, int number)
{
    return Integer::Bitfield(environment, APIntFactory(number));
}

static Integer::Enumeration
EnumerationFactory(const Environment &environment, int number)
{
    return Integer::Enumeration(environment, APIntFactory(number));
}

static void
testMergeInterval(const Environment &environment)
{
    Integer::Interval interval1 = IntervalFactory(environment, 0),
            interval2 = IntervalFactory(environment, 1),
            interval3 = IntervalFactory(environment, -1),
            interval4(interval1), interval5(interval2), interval6(interval3),result(interval1),
            interval7 = IntervalFactory(environment, 2), interval8 = IntervalFactory(environment, 3), interval9(interval7),
            interval10(interval1), interval11(interval1);
    llvm::APInt res;

    interval4.merge(interval2); //0-1
    CANAL_ASSERT(interval4.signedMin(res) && res == 0 && interval4.signedMax(res) && res == 1);

    interval4.merge(interval4); //Merge with itself
    CANAL_ASSERT(interval4.signedMin(res) && res == 0 && interval4.signedMax(res) && res == 1);

    interval5.merge(interval1); //0-1
    CANAL_ASSERT(interval5.signedMin(res) && res == 0 && interval5.signedMax(res) && res == 1);

    interval6.merge(interval1); //-1-0
    CANAL_ASSERT(interval6.signedMin(res) && res == APIntFactory(-1) && interval6.signedMax(res) && res == 0);
    CANAL_ASSERT(interval6.unsignedMin(res) && res == 0 && interval6.unsignedMax(res) && res == APIntFactory(-1));

    interval5.merge(interval6); //-1-1
    CANAL_ASSERT(interval5.signedMin(res) && res == APIntFactory(-1) && interval5.signedMax(res) && res == 1);
    CANAL_ASSERT(interval5.unsignedMin(res) && res == 0 && interval5.unsignedMax(res) && res == APIntFactory(-1)); //0-(-1)

    interval9.merge(interval8); //2-3
    CANAL_ASSERT(interval9.signedMin(res) && res == 2 && interval9.signedMax(res) && res == 3);

    interval6.merge(interval8); //-1-3
    CANAL_ASSERT(interval6.signedMin(res) && res == APIntFactory(-1) && interval6.signedMax(res) && res == 3);
    CANAL_ASSERT(interval6.unsignedMin(res) && res == 0 && interval6.unsignedMax(res) && res == APIntFactory(-1)); //0-(-1)

    interval10.setTop(); //Top
    CANAL_ASSERT(interval10.isTop());

    interval10.merge(interval10); //Merge with itself
    CANAL_ASSERT(interval10.isTop());

    interval10.merge(interval4); //Merge with any interval
    CANAL_ASSERT(interval10.isTop());

    for (unsigned i = 0; i < 1000; i ++) {
        interval11.merge(IntervalFactory(environment, i));
    }
    CANAL_ASSERT(interval11.signedMin(res) && res == 0 && interval11.signedMax(res) && res == 999);
}

static void
testMergeEnumeration(const Environment &environment)
{
    Integer::Enumeration enumeration1 = EnumerationFactory(environment, 0),
            enumeration2 = EnumerationFactory(environment, 1),
            enumeration3 = EnumerationFactory(environment, -1),
            enumeration4(enumeration1), enumeration5(enumeration2), enumeration6(enumeration3),result(enumeration1),
            enumeration7 = EnumerationFactory(environment, 2), enumeration8 = EnumerationFactory(environment, 3), enumeration9(enumeration7),
            enumeration10(enumeration1), enumeration11(enumeration1);
    llvm::APInt res;

    enumeration4.merge(enumeration2); //0-1
    CANAL_ASSERT(enumeration4.signedMin(res) && res == 0 && enumeration4.signedMax(res) && res == 1);

    enumeration4.merge(enumeration4); //Merge with itself
    CANAL_ASSERT(enumeration4.signedMin(res) && res == 0 && enumeration4.signedMax(res) && res == 1);

    enumeration5.merge(enumeration1); //0-1
    CANAL_ASSERT(enumeration5.signedMin(res) && res == 0 && enumeration5.signedMax(res) && res == 1);

    enumeration6.merge(enumeration1); //-1-0
    CANAL_ASSERT(enumeration6.signedMin(res) && res == APIntFactory(-1) && enumeration6.signedMax(res) && res == 0);
    CANAL_ASSERT(enumeration6.unsignedMin(res) && res == 0 && enumeration6.unsignedMax(res) && res == APIntFactory(-1));

    enumeration5.merge(enumeration6); //-1-1
    CANAL_ASSERT(enumeration5.signedMin(res) && res == APIntFactory(-1) && enumeration5.signedMax(res) && res == 1);
    CANAL_ASSERT(enumeration5.unsignedMin(res) && res == 0 && enumeration5.unsignedMax(res) && res == APIntFactory(-1)); //0-(-1)

    enumeration9.merge(enumeration8); //2-3
    CANAL_ASSERT(enumeration9.signedMin(res) && res == 2 && enumeration9.signedMax(res) && res == 3);

    enumeration6.merge(enumeration8); //-1-3
    CANAL_ASSERT(enumeration6.signedMin(res) && res == APIntFactory(-1) && enumeration6.signedMax(res) && res == 3);
    CANAL_ASSERT(enumeration6.unsignedMin(res) && res == 0 && enumeration6.unsignedMax(res) && res == APIntFactory(-1)); //0-(-1)

    enumeration10.setTop(); //Top
    CANAL_ASSERT(enumeration10.isTop());

    enumeration10.merge(enumeration10); //Merge with itself
    CANAL_ASSERT(enumeration10.isTop());

    enumeration10.merge(enumeration4); //Merge with any enumeration
    CANAL_ASSERT(enumeration10.isTop());

    //Test of overflow
    for (unsigned i = 0; i < 100000; i ++) {
        enumeration11.merge(EnumerationFactory(environment, i));
        if (enumeration11.isTop()) break;
    }
    CANAL_ASSERT(enumeration11.isTop());
}

static void
testMergeBitfield(const Environment &environment)
{
    Integer::Bitfield bitfield1 = BitfieldFactory(environment, 0),
            bitfield2 = BitfieldFactory(environment, 1),
            bitfield3 = BitfieldFactory(environment, -1),
            bitfield4(bitfield1), bitfield5(bitfield2), bitfield6(bitfield3),result(bitfield1),
            bitfield7 = BitfieldFactory(environment, 2), bitfield8 = BitfieldFactory(environment, 3), bitfield9(bitfield7),
            bitfield10(bitfield1), bitfield11(bitfield2);
    llvm::APInt res;

    bitfield4.merge(bitfield2); //0-1 -> ...00T
    CANAL_ASSERT(bitfield4.unsignedMin(res) && res == 0 && bitfield4.unsignedMax(res) && res == 1);
    CANAL_ASSERT(bitfield4.signedMin(res) && res == 0 && bitfield4.signedMax(res) && res == 1);

    bitfield4.merge(bitfield4); //Merge with itself
    CANAL_ASSERT(bitfield4.unsignedMin(res) && res == 0 && bitfield4.unsignedMax(res) && res == 1);
    CANAL_ASSERT(bitfield4.signedMin(res) && res == 0 && bitfield4.signedMax(res) && res == 1);

    bitfield5.merge(bitfield1); //0-1 -> ...00T
    CANAL_ASSERT(bitfield5.unsignedMin(res) && res == 0 && bitfield5.unsignedMax(res) && res == 1);
    CANAL_ASSERT(bitfield5.signedMin(res) && res == 0 && bitfield5.signedMax(res) && res == 1);

    bitfield6.merge(bitfield1); //-1-0 -> TOP
    CANAL_ASSERT(bitfield6.isTop());
    CANAL_ASSERT(bitfield6.signedMin(res) && res == llvm::APInt::getSignedMinValue(8*sizeof(int))
                 && bitfield6.signedMax(res) && res == llvm::APInt::getSignedMaxValue(8*sizeof(int)));
    CANAL_ASSERT(bitfield6.unsignedMin(res) && res == 0
                 && bitfield6.unsignedMax(res) && res == llvm::APInt::getMaxValue(8*sizeof(int)));

    bitfield9.merge(bitfield8); //2-3 -> ...01T
    CANAL_ASSERT(bitfield9.unsignedMin(res) && res == 2 && bitfield9.unsignedMax(res) && res == 3);
    CANAL_ASSERT(bitfield9.signedMin(res) && res == 2 && bitfield9.signedMax(res) && res == 3);

    bitfield5.merge(bitfield8); //0-3 -> ...0TT
    CANAL_ASSERT(bitfield5.signedMin(res) && res == 0 && bitfield5.signedMax(res) && res == 3);
    CANAL_ASSERT(bitfield5.unsignedMin(res) && res == 0 && bitfield5.unsignedMax(res) && res == 3);

    bitfield10.setTop(); //Top
    CANAL_ASSERT(bitfield10.isTop());

    bitfield10.merge(bitfield10); //Merge with itself
    CANAL_ASSERT(bitfield10.isTop());

    bitfield10.merge(bitfield4); //Merge with any bitfield
    CANAL_ASSERT(bitfield10.isTop());

    for (unsigned i = 0; i < 1024; i ++) {
        bitfield11.merge(BitfieldFactory(environment, i));
    }
    //0-1023 -> ...0TTTTTTTTTT (last ten bits is T)
    CANAL_ASSERT(bitfield11.signedMin(res) && res == 0 && bitfield11.signedMax(res) && res == 1023);
    CANAL_ASSERT(bitfield11.unsignedMin(res) && res == 0 && bitfield11.unsignedMax(res) && res == 1023);
}


int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    Environment environment(module);
    
    testMergeInterval(environment);
    testMergeEnumeration(environment);
    testMergeBitfield(environment);

    return 0;
}
