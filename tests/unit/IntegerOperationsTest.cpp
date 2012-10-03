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
testAddInterval(const Environment &environment)
{
    Integer::Interval interval1 = IntervalFactory(environment, 0),
            interval2 = IntervalFactory(environment, 1),
            interval3 = IntervalFactory(environment, -1),
            interval4(interval1), interval5(interval2), interval6(interval3),result(interval1),
            interval7 = IntervalFactory(environment, 2), interval8 = IntervalFactory(environment, 3), interval9(interval7),
            interval10(interval1), interval11(interval1), top(interval1);
    llvm::APInt res;

    interval4.merge(interval2); //0-1
    interval5.merge(interval1); //0-1
    interval6.merge(interval1); //-1-0
    interval9.merge(interval8); //2-3
    interval10.merge(interval7); //0-2
    interval11.merge(interval8); //1-3
    top.setTop();

    result.add(interval1, interval1); //0 + 0 = 0
    CANAL_ASSERT(result == interval1);

    result.add(result, result); //0 + 0 = 0
    CANAL_ASSERT(result == interval1);

    result.add(interval1, top); //0 + TOP = TOP
    CANAL_ASSERT(result.isTop());

    result.add(interval4, interval1); //0-1 + 0 = 0-1
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == 1);

    result.add(interval4, interval5); //0-1 + 0-1 = 0-2
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == 2);

    result.add(interval8, interval11); //2-3 + 1-3 = 3-6
    CANAL_ASSERT(result.signedMin(res) && res == 3 && result.signedMax(res) && res == 6);
    CANAL_ASSERT(result.unsignedMin(res) && res == 3 && result.unsignedMax(res) && res == 6);

    result.add(interval4, interval6); //0-1 + -1-0 = -1-1 / TOP for unsigned
    CANAL_ASSERT(result.signedMin(res) && res == APIntFactory(-1) && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.mUnsignedTop);

    result.add(result, interval6); //-1-1 + -1-0 = -2-1 / TOP for unsigned
    CANAL_ASSERT(result.signedMin(res) && res == APIntFactory(-2) && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.mUnsignedTop);

    result.add(result, Integer::Interval(environment, llvm::APInt::getSignedMinValue(8*sizeof(int)))); //-1-1 + -signedMin = TOP
    CANAL_ASSERT(result.isTop());

    result.add(result, interval1); //TOP + anything = TOP
    CANAL_ASSERT(result.isTop());

    result.add(result, interval5); //TOP + anything = TOP
    CANAL_ASSERT(result.isTop());

    result.add(interval1, interval1); //Reset result
    for (unsigned i = 0; i < 1000; i ++) {
        result.add(result, interval9); //2-3
    }
    CANAL_ASSERT(result.signedMin(res) && res == 2000 && result.signedMax(res) && res == 3000);
    CANAL_ASSERT(result.unsignedMin(res) && res == 2000 && result.unsignedMax(res) && res == 3000);

    for (unsigned i = 0; i < 2000; i ++) {
        result.add(result, interval6);
    }
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 3000);
    CANAL_ASSERT(result.mUnsignedTop);
}

static void
testAddEnumeration(const Environment &environment)
{
    Integer::Enumeration enumeration1 = EnumerationFactory(environment, 0),
            enumeration2 = EnumerationFactory(environment, 1),
            enumeration3 = EnumerationFactory(environment, -1),
            enumeration4(enumeration1), enumeration5(enumeration2), enumeration6(enumeration3),result(enumeration1),
            enumeration7 = EnumerationFactory(environment, 2), enumeration8 = EnumerationFactory(environment, 3), enumeration9(enumeration7),
            enumeration10(enumeration1), enumeration11(enumeration1), top(enumeration1);
    llvm::APInt res;

    enumeration4.merge(enumeration2); //0-1
    enumeration5.merge(enumeration1); //0-1
    enumeration6.merge(enumeration1); //-1-0
    enumeration9.merge(enumeration8); //2-3
    enumeration10.merge(enumeration7); //0-2
    enumeration11.merge(enumeration8); //1-3
    top.setTop();

    result.add(enumeration1, enumeration1); //0 + 0 = 0
    CANAL_ASSERT(result == enumeration1);

    result.add(enumeration1, top); //0 + TOP = TOP
    CANAL_ASSERT(result.isTop());

    result.add(enumeration4, enumeration1); //0-1 + 0 = 0-1
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == 1);

    result.add(enumeration4, enumeration5); //0-1 + 0-1 = 0-2
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == 2);

    result.add(enumeration8, enumeration11); //2-3 + 1-3 = 3-6
    CANAL_ASSERT(result.signedMin(res) && res == 3 && result.signedMax(res) && res == 6);
    CANAL_ASSERT(result.unsignedMin(res) && res == 3 && result.unsignedMax(res) && res == 6);

    result.add(enumeration4, enumeration6); //0-1 + -1-0 = -1-1 / TOP for unsigned
    CANAL_ASSERT(result.signedMin(res) && res == APIntFactory(-1) && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == APIntFactory(-1));

    Integer::Enumeration tmp1(result);
    result.add(tmp1, enumeration6); //-1-1 + -1-0 = -2-1 / TOP for unsigned
    CANAL_ASSERT(result.signedMin(res) && res == APIntFactory(-2) && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == APIntFactory(-1));

    result.add(top, enumeration1); //TOP + anything = TOP
    CANAL_ASSERT(result.isTop());

    result.add(top, enumeration5); //TOP + anything = TOP
    CANAL_ASSERT(result.isTop());

    for (unsigned i = 0; i < 30; i ++) {
        Integer::Enumeration tmp(i ? result : enumeration9);
        result.add(tmp, enumeration9); //2-3
    }
    CANAL_ASSERT(result.signedMin(res) && res == 2*31 && result.signedMax(res) && res == 3*31);
    CANAL_ASSERT(result.unsignedMin(res) && res == 2*31 && result.unsignedMax(res) && res == 3*31);

    for (unsigned i = 0; i < 30; i ++) {
        Integer::Enumeration tmp(result);
        result.add(tmp, enumeration6);
    }
    //CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 3000);
    CANAL_ASSERT(result.isTop());
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
    
    testAddInterval(environment);
    testAddEnumeration(environment);
    //testMergeBitfield(environment);

    return 0;
}
