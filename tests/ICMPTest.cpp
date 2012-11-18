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

static Integer::Interval
IntervalFactory(const Environment &environment, int number)
{
    return Integer::Interval(environment, llvm::APInt(sizeof(int)*8, number, number < 0));
}

static Integer::Bitfield
BitfieldFactory(const Environment &environment, int number)
{
    return Integer::Bitfield(environment, llvm::APInt(sizeof(int)*8, number, number < 0));
}

static Integer::Enumeration
EnumerationFactory(const Environment &environment, int number)
{
    return Integer::Enumeration(environment, llvm::APInt(sizeof(int)*8, number, number < 0));
}

static void
testEqInterval(const Environment &environment)
{
    Integer::Interval interval1 = IntervalFactory(environment, 0),
            interval2 = IntervalFactory(environment, 1),
            interval3 = IntervalFactory(environment, -1),
            interval4(interval1), interval5(interval2), interval6(interval3),result(interval1),
            interval7 = IntervalFactory(environment, 2), interval8 = IntervalFactory(environment, 3), interval9(interval7),
            interval10(interval1), interval11(interval2);
    llvm::APInt res;
    interval4.merge(interval2); //0-1
    interval5.merge(interval1); //0-1
    interval6.merge(interval1); //-1-0
    interval9.merge(interval8); //2-3
    interval10.merge(interval7); //0-2
    interval11.merge(interval8); //1-3
	
    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_EQ); //0 != 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_NE); //0 != 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);
	
    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_EQ); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> signed false, unsigned top
    CANAL_ASSERT(result.isSignedSingleValue() && result.signedMin(res) && res == 0); //Signed false
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_NE); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> signed true, unsigned top
    CANAL_ASSERT(result.isSignedSingleValue() && result.signedMin(res) && res == 1); //Signed true
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_EQ); //0-1 != 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_NE); //0-1 != 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_EQ); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_NE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_EQ); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_NE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_EQ); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_NE); //0-1 ==  0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_EQ); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_NE); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval4, llvm::CmpInst::ICMP_EQ); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval4, llvm::CmpInst::ICMP_NE); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_EQ); //0-1 != 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_NE); //0-1 != 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_EQ); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_NE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());
}

static void
testEqBitfield(const Environment &environment)
{
    Integer::Bitfield bitfield1 = BitfieldFactory(environment, 0),
            bitfield2 = BitfieldFactory(environment, 1),
            bitfield3 = BitfieldFactory(environment, -1),
            bitfield4(bitfield1), bitfield5(bitfield2), bitfield6(bitfield3),result(bitfield1),
            bitfield7 = BitfieldFactory(environment, 2), bitfield8 = BitfieldFactory(environment, 3), bitfield9(bitfield7),
            bitfield10(bitfield1), bitfield11(bitfield2);
    llvm::APInt res;
    bitfield4.merge(bitfield2); //0-1
    bitfield5.merge(bitfield1); //0-1
    bitfield6.merge(bitfield1); //-1-0
    bitfield9.merge(bitfield8); //2-3
    bitfield10.merge(bitfield7); //0-2
    bitfield11.merge(bitfield8); //1-3

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_EQ); //0 != 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_NE); //0 != 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_EQ); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> unsigned top
    CANAL_ASSERT(result.isTop()); //Difference from interval

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_NE); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> signed true, unsigned top
    CANAL_ASSERT(result.isTop()); //Difference from interval

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_EQ); //0-1 != 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_NE); //0-1 != 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_EQ); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_NE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_EQ); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_NE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_EQ); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_NE); //0-1 ==  0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_EQ); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_NE); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield4, llvm::CmpInst::ICMP_EQ); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield4, llvm::CmpInst::ICMP_NE); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_EQ); //0-1 != 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_NE); //0-1 != 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    //Difference from interval
    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_EQ); //0-2 != 1-3 (difference in last bit)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_NE); //0-2 != 1-3 (difference in last bit)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);
}

static void
testEqEnumeration(const Environment &environment)
{
    Integer::Enumeration enumeration1 = EnumerationFactory(environment, 0),
            enumeration2 = EnumerationFactory(environment, 1),
            enumeration3 = EnumerationFactory(environment, -1),
            enumeration4(enumeration1), enumeration5(enumeration2), enumeration6(enumeration3),result(enumeration1),
            enumeration7 = EnumerationFactory(environment, 2), enumeration8 = EnumerationFactory(environment, 3), enumeration9(enumeration7),
            enumeration10(enumeration1), enumeration11(enumeration2);
    llvm::APInt res;
    enumeration4.merge(enumeration2); //0-1
    enumeration5.merge(enumeration1); //0-1
    enumeration6.merge(enumeration1); //-1-0
    enumeration9.merge(enumeration8); //2-3
    enumeration10.merge(enumeration7); //0-2
    enumeration11.merge(enumeration8); //1-3

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_EQ); //0 != 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_NE); //0 != 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    //Difference from interval
    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_EQ); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> false
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0); //False


    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_NE); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> true
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1); //True

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_EQ); //0-1 != 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_NE); //0-1 != 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_EQ); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_NE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_EQ); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_NE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_EQ); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_NE); //0-1 ==  0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_EQ); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_NE); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration4, llvm::CmpInst::ICMP_EQ); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration4, llvm::CmpInst::ICMP_NE); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_EQ); //0-1 != 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_NE); //0-1 != 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    //Difference from interval
    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_EQ); //0-2 != 1-3 (no intersection)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_NE); //0-2 != 1-3 (no intersection)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);
}

static void
testIneqInterval(const Environment &environment)
{
    Integer::Interval interval1 = IntervalFactory(environment, 0),
            interval2 = IntervalFactory(environment, 1),
            interval3 = IntervalFactory(environment, -1),
            interval4(interval1), interval5(interval2), interval6(interval3),result(interval1),
            interval7 = IntervalFactory(environment, 2), interval8 = IntervalFactory(environment, 3), interval9(interval7),
            interval10(interval1), interval11(interval2);
    llvm::APInt res;
    interval4.merge(interval2); //0-1
    interval5.merge(interval1); //0-1
    interval6.merge(interval1); //-1-0
    interval9.merge(interval8); //2-3
    interval10.merge(interval7); //0-2
    interval11.merge(interval8); //1-3

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_SGT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_SGE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_UGT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_UGE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_SLT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_SLE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_ULT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval1, interval2, llvm::CmpInst::ICMP_ULE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_SLE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_SLT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_ULE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_ULT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_SGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_SGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_UGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_UGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());


    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_SLE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_SLT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_ULE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_ULT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_SGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_SGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_UGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_UGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());


    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_SGE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_SGT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_UGE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_UGT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_SLE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_SLT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_ULE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_ULT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_SGE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_SGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_UGE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_UGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_SLE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_SLT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_ULE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_ULT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());


    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_UGT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_UGE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_SGT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_SGE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_ULT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_ULE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_SLT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_SLE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());


    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_UGT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_UGE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_SGT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_SGE); //0-1 <= -1-0 //Signed
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_ULT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_ULE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_SLT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_SLE); //0-1 <= -1-0 //Signed
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);


    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_SGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_SGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_UGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_UGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_SLT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_SLE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_ULT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_ULE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_SGE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_SGT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_UGE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_UGT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_SLE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_SLT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_ULE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_ULT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_SGE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_SGT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_UGE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_UGT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_SLE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_SLT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_ULE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_ULT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());
}

static void
testIneqEnumeration(const Environment &environment)
{
    Integer::Enumeration enumeration1 = EnumerationFactory(environment, 0),
            enumeration2 = EnumerationFactory(environment, 1),
            enumeration3 = EnumerationFactory(environment, -1),
            enumeration4(enumeration1), enumeration5(enumeration2), enumeration6(enumeration3),result(enumeration1),
            enumeration7 = EnumerationFactory(environment, 2), enumeration8 = EnumerationFactory(environment, 3), enumeration9(enumeration7),
            enumeration10(enumeration1), enumeration11(enumeration2);
    llvm::APInt res;
    enumeration4.merge(enumeration2); //0-1
    enumeration5.merge(enumeration1); //0-1
    enumeration6.merge(enumeration1); //-1-0
    enumeration9.merge(enumeration8); //2-3
    enumeration10.merge(enumeration7); //0-2
    enumeration11.merge(enumeration8); //1-3

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_SGT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_SGE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_UGT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_UGE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_SLT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_SLE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_ULT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_ULE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_SLE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_SLT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_ULE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_ULT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_SGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_SGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_UGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_UGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());


    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_SLE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_SLT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_ULE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_ULT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_SGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_SGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_UGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_UGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());


    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_SGE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_SGT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_UGE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_UGT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_SLE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_SLT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_ULE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_ULT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_SGE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_SGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_UGE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_UGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_SLE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_SLT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_ULE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_ULT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_UGT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_UGE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_SGT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_SGE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_ULT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_ULE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_SLT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_SLE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());


    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_UGT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_UGE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_SGT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_SGE); //0-1 <= -1-0 //Signed
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_ULT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_ULE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_SLT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_SLE); //0-1 <= -1-0 //Signed
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);


    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_SGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_SGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_UGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_UGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_SLT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_SLE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_ULT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_ULE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_SGE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_SGT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_UGE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_UGT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_SLE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_SLT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_ULE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_ULT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_SGE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_SGT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_UGE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_UGT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_SLE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_SLT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_ULE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_ULT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());
}

static void
testIneqBitfield(const Environment &environment)
{
    Integer::Bitfield bitfield1 = BitfieldFactory(environment, 0),
            bitfield2 = BitfieldFactory(environment, 1),
            bitfield3 = BitfieldFactory(environment, -1),
            bitfield4(bitfield1), bitfield5(bitfield2), bitfield6(bitfield3),result(bitfield1),
            bitfield7 = BitfieldFactory(environment, 2), bitfield8 = BitfieldFactory(environment, 3), bitfield9(bitfield7),
            bitfield10(bitfield1), bitfield11(bitfield2);
    llvm::APInt res;
    bitfield4.merge(bitfield2); //0-1
    bitfield5.merge(bitfield1); //0-1
    bitfield6.merge(bitfield1); //-1-0
    bitfield9.merge(bitfield8); //2-3
    bitfield10.merge(bitfield7); //0-2
    bitfield11.merge(bitfield8); //1-3

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_SGT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_SGE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_UGT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_UGE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_SLT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_SLE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_ULT); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield1, bitfield2, llvm::CmpInst::ICMP_ULE); //0 > 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_SLE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_SLT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_ULE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_ULT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_SGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_SGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_UGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield2, bitfield6, llvm::CmpInst::ICMP_UGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());


    result.icmp(bitfield1, bitfield6, llvm::CmpInst::ICMP_SLE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield6, llvm::CmpInst::ICMP_SLT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield6, llvm::CmpInst::ICMP_ULE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield6, llvm::CmpInst::ICMP_ULT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield6, llvm::CmpInst::ICMP_SGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield6, llvm::CmpInst::ICMP_SGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield6, llvm::CmpInst::ICMP_UGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield1, bitfield6, llvm::CmpInst::ICMP_UGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());


    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_SGE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_SGT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_UGE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_UGT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_SLE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_SLT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_ULE); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield7, llvm::CmpInst::ICMP_ULT); //0-1 < 2
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_SGE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_SGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_UGE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_UGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_SLE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_SLT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_ULE); //0-1 <= 1
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield1, llvm::CmpInst::ICMP_ULT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());


    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_UGT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_UGE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_SGT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_SGE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_ULT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_ULE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_SLT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield5, llvm::CmpInst::ICMP_SLE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());


    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_UGT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_UGE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_SGT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_SGE); //0-1 <= -1-0 //Signed
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_ULT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_ULE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_SLT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield6, llvm::CmpInst::ICMP_SLE); //0-1 <= -1-0 //Signed
    //CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.isTop());


    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_SGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_SGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_UGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_UGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_SLT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_SLE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_ULT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield4, bitfield4, llvm::CmpInst::ICMP_ULE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_SGE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_SGT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_UGE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_UGT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 0);

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_SLE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_SLT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_ULE); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);

    result.icmp(bitfield4, bitfield9, llvm::CmpInst::ICMP_ULT); //0-1 < 2-3
    CANAL_ASSERT(result.isSingleValue() && result.signedMin(res) && res == 1);


    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_SGE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_SGT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_UGE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_UGT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_SLE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_SLT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_ULE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(bitfield10, bitfield11, llvm::CmpInst::ICMP_ULT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());
}


int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    Environment environment(module);
    
    testEqInterval(environment);
    testEqBitfield(environment);
    testEqEnumeration(environment);

    testIneqInterval(environment);
    testIneqEnumeration(environment);
    testIneqBitfield(environment);

    return 0;
}
