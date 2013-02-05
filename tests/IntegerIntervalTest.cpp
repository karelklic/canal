#include "lib/IntegerInterval.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>
#include "lib/FloatInterval.h"

using namespace Canal;

static Environment *gEnvironment;

static void
testConstructors()
{
    Integer::Interval interval(*gEnvironment, 1);
    CANAL_ASSERT(interval.isBottom());
    CANAL_ASSERT(!interval.isTop());
    CANAL_ASSERT(interval.mSignedFrom == interval.mSignedTo);
    CANAL_ASSERT(interval.mUnsignedFrom == interval.mUnsignedTo);
    CANAL_ASSERT(interval.mSignedFrom.getSExtValue() == 0);
    CANAL_ASSERT(interval.mUnsignedFrom.getZExtValue() == 0);
}

static void
testEquality()
{
    Integer::Interval interval1(*gEnvironment, /*bitWidth=*/1);
    Integer::Interval interval2(*gEnvironment, 1);
    Integer::Interval interval3(*gEnvironment, 2);

    // Test empty intervals.
    CANAL_ASSERT(interval1 == interval2);
    CANAL_ASSERT(interval1 != interval3);
}

static void
testJoin()
{
    llvm::APInt zero(32, 0), one(32, 1), two(32, 2), three(32, 3),
        negone(32, -1, true);

    Integer::Interval
        interval1 = Integer::Interval(*gEnvironment, zero),
        interval2 = Integer::Interval(*gEnvironment, one),
        interval3 = Integer::Interval(*gEnvironment, negone),
        interval4(interval1),
        interval5(interval2),
        interval6(interval3),
        result(interval1),
        interval7 = Integer::Interval(*gEnvironment, two),
        interval8 = Integer::Interval(*gEnvironment, three),
        interval9(interval7),
        interval10(interval1),
        interval11(interval1);

    llvm::APInt res;

    interval4.join(interval2); //0-1
    CANAL_ASSERT(interval4.signedMin(res) && res == 0 && interval4.signedMax(res) && res == 1);

    interval4.join(interval4); //Join with itself
    CANAL_ASSERT(interval4.signedMin(res) && res == 0 && interval4.signedMax(res) && res == 1);

    interval5.join(interval1); //0-1
    CANAL_ASSERT(interval5.signedMin(res) && res == 0 && interval5.signedMax(res) && res == 1);

    interval6.join(interval1); //-1-0
    CANAL_ASSERT(interval6.signedMin(res) && res == negone && interval6.signedMax(res) && res == 0);
    CANAL_ASSERT(interval6.unsignedMin(res) && res == 0 && interval6.unsignedMax(res) && res == negone);

    interval5.join(interval6); //-1-1
    CANAL_ASSERT(interval5.signedMin(res) && res == negone && interval5.signedMax(res) && res == 1);
    CANAL_ASSERT(interval5.unsignedMin(res) && res == 0 && interval5.unsignedMax(res) && res == negone); //0-(-1)

    interval9.join(interval8); //2-3
    CANAL_ASSERT(interval9.signedMin(res) && res == 2 && interval9.signedMax(res) && res == 3);

    interval6.join(interval8); //-1-3
    CANAL_ASSERT(interval6.signedMin(res) && res == negone && interval6.signedMax(res) && res == 3);
    CANAL_ASSERT(interval6.unsignedMin(res) && res == 0 && interval6.unsignedMax(res) && res == negone); //0-(-1)

    interval10.setTop(); //Top
    CANAL_ASSERT(interval10.isTop());

    interval10.join(interval10); //Join with itself
    CANAL_ASSERT(interval10.isTop());

    interval10.join(interval4); //Join with any interval
    CANAL_ASSERT(interval10.isTop());

    for (unsigned i = 0; i < 1000; i ++) {
        interval11.join(Integer::Interval(*gEnvironment, llvm::APInt(32, i)));
    }
    CANAL_ASSERT(interval11.signedMin(res) && res == 0 && interval11.signedMax(res) && res == 999);
}

static void
testMeet()
{
    Integer::Interval
        zero(*gEnvironment, llvm::APInt(32, 0)),
        one(*gEnvironment, llvm::APInt(32, 1)),
        negone(*gEnvironment, llvm::APInt(32, -1, true)),
        ten(*gEnvironment, llvm::APInt(32, 10)),
        unsigned_one(*gEnvironment, 32),
        signed_one(*gEnvironment, 32),
        bottom(*gEnvironment, 32),
        top(*gEnvironment, 32),
        negone_one(negone),
        zero_one(zero);

    top.setTop();
    negone_one.join(one);
    zero_one.join(one);

    llvm::APInt res;
    unsigned_one.udiv(ten, ten);
    CANAL_ASSERT(unsigned_one.isSignedTop() && unsigned_one.isUnsignedConstant() && unsigned_one.unsignedMin(res) && res == 1);
    signed_one.sdiv(ten, ten);
    CANAL_ASSERT(signed_one.isUnsignedTop() && signed_one.isSignedConstant() && signed_one.signedMin(res) && res == 1);

    //Top and bottom
    CANAL_ASSERT(bottom.meet(bottom).isBottom());
    CANAL_ASSERT(top.meet(top).isTop());
    {
        Integer::Interval result(top), result1(bottom), result2(zero),
               unsigned_one1(unsigned_one), signed_one1(signed_one);
        CANAL_ASSERT(result.meet(bottom).isBottom());
        CANAL_ASSERT(result1.meet(top).isBottom());
        CANAL_ASSERT(result2.meet(top) == zero);
        CANAL_ASSERT(result2.meet(bottom).isBottom());

        CANAL_ASSERT(unsigned_one1.meet(top) == unsigned_one);
        CANAL_ASSERT(unsigned_one1.meet(bottom).isBottom());
        CANAL_ASSERT(signed_one1.meet(top) == signed_one);
        CANAL_ASSERT(signed_one1.meet(bottom).isBottom());
    }

    //Tests with values
    {
        Integer::Interval zero1(zero), one1(one), negone1(negone), negone2(negone),
                unsigned_one1(unsigned_one), signed_one1(signed_one), signed_one2(signed_one),
                negone_one1(negone_one), negone_one2(negone_one), zero_one1(zero_one);
        CANAL_ASSERT(zero1.meet(zero) == zero);
        CANAL_ASSERT(one1.meet(one) == one);
        CANAL_ASSERT(negone1.meet(negone) == negone);
        CANAL_ASSERT(unsigned_one1.meet(unsigned_one) == unsigned_one);
        CANAL_ASSERT(signed_one1.meet(signed_one) == signed_one);
        CANAL_ASSERT(negone_one1.meet(negone_one) == negone_one);
        CANAL_ASSERT(zero_one1.meet(zero_one) == zero_one);

        CANAL_ASSERT(zero1.meet(one).isBottom());
        CANAL_ASSERT(one1.meet(zero_one) == one);
        CANAL_ASSERT(one1.meet(negone_one) == one);

        CANAL_ASSERT(negone_one1.meet(one) == one);
        CANAL_ASSERT(negone_one2.meet(zero_one).isUnsignedConstant()
                     && negone_one2.signedMin(res) && res == 0
                     && negone_one2.signedMax(res) && res == 1); //Unsigned 1, signed 0 - 1
        CANAL_ASSERT(zero_one1.meet(negone_one) == negone_one2);

        CANAL_ASSERT(negone1.meet(negone_one) == negone);
        CANAL_ASSERT(negone1.meet(signed_one).isSignedBottom()
                     && negone1.isUnsignedConstant()
                     && negone1.unsignedMin(res) && res == llvm::APInt(32, -1, true));
        CANAL_ASSERT(signed_one1.meet(negone) == negone1);
        CANAL_ASSERT(negone2.meet(unsigned_one).isUnsignedBottom()
                     && negone2.isSignedConstant()
                     && negone2.signedMin(res) && res == llvm::APInt(32, -1, true));
        CANAL_ASSERT(unsigned_one1.meet(negone) == negone2);

        CANAL_ASSERT(signed_one2.meet(unsigned_one) == one);
    }
}

static void
testInclusion()
{
    Integer::Interval
        zero(*gEnvironment, llvm::APInt(32, 0)),
        one(*gEnvironment, llvm::APInt(32, 1)),
        negone(*gEnvironment, llvm::APInt(32, -1, true)),
        ten(*gEnvironment, llvm::APInt(32, 10)),
        unsigned_one(*gEnvironment, 32),
        signed_one(*gEnvironment, 32),
        bottom(*gEnvironment, 32),
        top(*gEnvironment, 32),
        negone_one(negone),
        zero_one(zero);

    top.setTop();
    negone_one.join(one);
    zero_one.join(one);

    llvm::APInt res;
    unsigned_one.udiv(ten, ten); //Signed top, unsigned one
    CANAL_ASSERT(unsigned_one.isSignedTop() && unsigned_one.isUnsignedConstant() && unsigned_one.unsignedMin(res) && res == 1);
    signed_one.sdiv(ten, ten); //Unsigned top, signed one
    CANAL_ASSERT(signed_one.isUnsignedTop() && signed_one.isSignedConstant() && signed_one.signedMin(res) && res == 1);

    //Top and bottom
    CANAL_ASSERT(bottom < bottom);
    CANAL_ASSERT(top < top);
    CANAL_ASSERT(!(top < bottom));
    CANAL_ASSERT(!(zero < bottom));
    CANAL_ASSERT(bottom < zero);
    CANAL_ASSERT(zero < top);
    CANAL_ASSERT(!(top < zero));

    CANAL_ASSERT(unsigned_one < top);
    CANAL_ASSERT(!(unsigned_one < bottom));
    CANAL_ASSERT(signed_one < top);
    CANAL_ASSERT(!(signed_one < bottom));

    //Tests with values
    CANAL_ASSERT(zero < zero);
    CANAL_ASSERT(one < one);
    CANAL_ASSERT(negone < negone);
    CANAL_ASSERT(unsigned_one < unsigned_one);
    CANAL_ASSERT(signed_one < signed_one);
    CANAL_ASSERT(negone_one < negone_one);
    CANAL_ASSERT(zero_one < zero_one);

    CANAL_ASSERT(!(zero < one));
    CANAL_ASSERT(one < zero_one);
    CANAL_ASSERT(!(zero_one < one));
    CANAL_ASSERT(one < negone_one);
    CANAL_ASSERT(!(negone_one < one));

    CANAL_ASSERT(!(negone_one < zero_one));
    CANAL_ASSERT(!(zero_one < negone_one)); //Works in signed, not in unsigned

    CANAL_ASSERT(negone < negone_one);
    CANAL_ASSERT(!(negone_one < negone));
    CANAL_ASSERT(!(negone < signed_one));
    CANAL_ASSERT(!(signed_one < negone));
    CANAL_ASSERT(!(negone < unsigned_one));
    CANAL_ASSERT(!(unsigned_one < negone));

    CANAL_ASSERT(!(signed_one < unsigned_one));
    CANAL_ASSERT(!(unsigned_one < signed_one));
}

static void
testIcmp()
{
    llvm::APInt zero(32, 0), one(32, 1);

    Integer::Interval result(*gEnvironment, 1);
    Integer::Interval const0(*gEnvironment, zero);
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_EQ).isTrue());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_NE).isFalse());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_UGE).isTrue());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_ULT).isFalse());
    CANAL_ASSERT(result.icmp(const0, const0, llvm::CmpInst::ICMP_ULE).isTrue());

    Integer::Interval const1(*gEnvironment, one);
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_SGE).isFalse());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_UGE).isFalse());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_SLT).isTrue());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_ULT).isTrue());
    CANAL_ASSERT(result.icmp(const0, const1, llvm::CmpInst::ICMP_ULE).isTrue());

    Integer::Interval from0to1(*gEnvironment, zero, one);
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(const0, from0to1, llvm::CmpInst::ICMP_ULE).isTrue());

    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_UGE).isTrue());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_ULT).isFalse());
    CANAL_ASSERT(result.icmp(from0to1, const0, llvm::CmpInst::ICMP_ULE).isTop());

    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(from0to1, from0to1, llvm::CmpInst::ICMP_ULE).isTop());

/*
        from1to1(gEnvironment, 1, 1),
        fromm1tom1(gEnvironment, -1, -1),
        from0to1(gEnvironment, 0, 1);

, interval5(interval2), interval6(interval3),result(interval1),
            interval7 = IntervalFactory(environment, 2), interval8 = IntervalFactory(environment, 3), interval9(interval7),
            interval10(interval1), interval11(interval2);
    llvm::APInt res;
    interval4.join(interval2); //0-1
    interval5.join(interval1); //0-1
    interval6.join(interval1); //-1-0
    interval9.join(interval8); //2-3
    interval10.join(interval7); //0-2
    interval11.join(interval8); //1-3

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_EQ); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> signed false, unsigned top
    CANAL_ASSERT(result.isSignedConstant() && result.signedMin(res) && res == 0); //Signed false
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_NE); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> signed true, unsigned top
    CANAL_ASSERT(result.isSignedConstant() && result.signedMin(res) && res == 1); //Signed true
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_EQ); //0-1 != 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_NE); //0-1 != 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_EQ); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval5, llvm::CmpInst::ICMP_NE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_EQ); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_NE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_EQ); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_NE); //0-1 ==  0-1 (same object)
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_EQ); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_NE); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval4, llvm::CmpInst::ICMP_EQ); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval4, llvm::CmpInst::ICMP_NE); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_EQ); //0-1 != 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_NE); //0-1 != 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_EQ); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(interval10, interval11, llvm::CmpInst::ICMP_NE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    Integer::Interval interval1 = IntervalFactory(environment, 0),
            interval2 = IntervalFactory(environment, 1),
            interval3 = IntervalFactory(environment, -1),
            interval4(interval1), interval5(interval2), interval6(interval3),result(interval1),
            interval7 = IntervalFactory(environment, 2), interval8 = IntervalFactory(environment, 3), interval9(interval7),
            interval10(interval1), interval11(interval2);
    llvm::APInt res;
    interval4.join(interval2); //0-1
    interval5.join(interval1); //0-1
    interval6.join(interval1); //-1-0
    interval9.join(interval8); //2-3
    interval10.join(interval7); //0-2
    interval11.join(interval8); //1-3

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_SLE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_SLT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_ULE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_ULT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_SGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_SGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_UGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(interval2, interval6, llvm::CmpInst::ICMP_UGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());


    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_SLE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_SLT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_ULE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_ULT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_SGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_SGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_UGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval1, interval6, llvm::CmpInst::ICMP_UGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());


    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_SGE); //0-1 < 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_SGT); //0-1 < 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_UGE); //0-1 < 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_UGT); //0-1 < 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_SLE); //0-1 < 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_SLT); //0-1 < 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_ULE); //0-1 < 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval7, llvm::CmpInst::ICMP_ULT); //0-1 < 2
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);


    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_SGE); //0-1 <= 1
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_SGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_UGE); //0-1 <= 1
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_UGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_SLE); //0-1 <= 1
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_SLT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval1, llvm::CmpInst::ICMP_ULE); //0-1 <= 1
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

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
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_ULT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_ULE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_SLT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval6, llvm::CmpInst::ICMP_SLE); //0-1 <= -1-0 //Signed
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);


    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_SGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_SGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_UGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_UGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_SLT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_SLE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_ULT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(interval4, interval4, llvm::CmpInst::ICMP_ULE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);


    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_SGE); //0-1 < 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_SGT); //0-1 < 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_UGE); //0-1 < 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_UGT); //0-1 < 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 0);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_SLE); //0-1 < 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_SLT); //0-1 < 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_ULE); //0-1 < 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);

    result.icmp(interval4, interval9, llvm::CmpInst::ICMP_ULT); //0-1 < 2-3
    CANAL_ASSERT(result.isConstant() && result.signedMin(res) && res == 1);


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
*/
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
    CANAL_ASSERT(interval2.isConstant() && interval2.unsignedMin(res) && res == 4);
    CANAL_ASSERT(interval2.getBitWidth() == 3);

    // Test truncation from i32 00000001 to i1.
    Integer::Interval interval3(*gEnvironment, 1);
    interval3.trunc(Integer::Interval(*gEnvironment, llvm::APInt(32, 1)));
    CANAL_ASSERT(interval3.isConstant() && interval3.unsignedMin(res) && res == 1);
    CANAL_ASSERT(interval3.getBitWidth() == 1);

    // Test truncation from i64 0 - 4294967295 (max i32 unsigned value) to i32 -> unsigned 0 to 4294967295, signed -1 to 0
    Integer::Interval interval4(*gEnvironment, 32);
    interval4.trunc(Integer::Interval(*gEnvironment, llvm::APInt(64, 4294967295u))
                    .join(Integer::Interval(*gEnvironment, llvm::APInt(64, 0))));
    CANAL_ASSERT(interval4.unsignedMin(res) && res == llvm::APInt(32, 0) && //Unsigned 0 to 4294967295
                 interval4.unsignedMax(res) && res == llvm::APInt(32, 4294967295u));
    CANAL_ASSERT(interval4.signedMin(res) && res == llvm::APInt(32, -1) && //Signed -1 to 0
                 interval4.signedMax(res) && res == llvm::APInt(32, 0));
    CANAL_ASSERT(interval4.getBitWidth() == 32);

    // Test truncation from i64 -1 - 0 to i32 -> unsigned top, signed -1 to 0
    Integer::Interval interval5(*gEnvironment, 32);
    interval5.trunc(Integer::Interval(*gEnvironment, llvm::APInt(64, -1, true))
                    .join(Integer::Interval(*gEnvironment, llvm::APInt(64, 0))));
    CANAL_ASSERT(interval5.unsignedMin(res) && res == llvm::APInt::getMinValue(32) && //Unsigned top
                 interval5.unsignedMax(res) && res == llvm::APInt::getMaxValue(32));
    CANAL_ASSERT(interval5.signedMin(res) && res == llvm::APInt(32, -1) && //Signed -1 to 0
                 interval5.signedMax(res) && res == llvm::APInt(32, 0));
    CANAL_ASSERT(interval5.getBitWidth() == 32);
}

static void
testAdd()
{
    Integer::Interval zero(*gEnvironment, llvm::APInt(32, 0)),
            one(*gEnvironment, llvm::APInt(32, 1)),
            minusone(*gEnvironment, llvm::APInt(32, -1, true)),
            zero_one(zero), zero_one2(one), minusone_zero(minusone), result(zero),
            two(*gEnvironment, llvm::APInt(32, 2)),
            three(*gEnvironment, llvm::APInt(32, 3)),
            two_three(two),
            zero_two(zero), one_three(zero), top(zero);
    llvm::APInt res;

    zero_one.join(one); //0-1
    zero_one2.join(zero); //0-1
    minusone_zero.join(zero); //-1-0
    two_three.join(three); //2-3
    zero_two.join(two); //0-2
    one_three.join(three); //1-3
    top.setTop();

    result.add(zero, zero); //0 + 0 = 0
    CANAL_ASSERT(result == zero);

    result.add(result, result); //0 + 0 = 0
    CANAL_ASSERT(result == zero);

    result.add(zero, top); //0 + TOP = TOP
    CANAL_ASSERT(result.isTop());

    result.add(zero_one, zero); //0-1 + 0 = 0-1
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == 1);

    result.add(zero_one, zero_one2); //0-1 + 0-1 = 0-2
    CANAL_ASSERT(result == zero_two);
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == 2);

    result.add(three, one_three); //2-3 + 1-3 = 3-6
    CANAL_ASSERT(result.signedMin(res) && res == 3 && result.signedMax(res) && res == 6);
    CANAL_ASSERT(result.unsignedMin(res) && res == 3 && result.unsignedMax(res) && res == 6);

    result.add(zero_one, minusone_zero); //0-1 + -1-0 = -1-1 / TOP for unsigned
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -1, true) && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.add(result, minusone_zero); //-1-1 + -1-0 = -2-1 / TOP for unsigned
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -2, true) && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.add(result, Integer::Interval(*gEnvironment, llvm::APInt::getSignedMinValue(32))); //-1-1 + -signedMin = TOP
    CANAL_ASSERT(result.isTop());

    result.add(result, zero); //TOP + anything = TOP
    CANAL_ASSERT(result.isTop());

    result.add(result, zero_one2); //TOP + anything = TOP
    CANAL_ASSERT(result.isTop());

    result.add(zero, zero); //Reset result
    for (unsigned i = 0; i < 1000; i ++) {
        result.add(result, two_three); //2-3
    }
    CANAL_ASSERT(result.signedMin(res) && res == 2000 && result.signedMax(res) && res == 3000);
    CANAL_ASSERT(result.unsignedMin(res) && res == 2000 && result.unsignedMax(res) && res == 3000);

    for (unsigned i = 0; i < 2000; i ++) {
        result.add(result, minusone_zero);
    }
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 3000);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));
}

static void
testDivisionByZero() {
    Integer::Interval zero(*gEnvironment, llvm::APInt(32, 0)),
            one(*gEnvironment, llvm::APInt(32, 1)),
            two(*gEnvironment, llvm::APInt(32, 2)),
            one_two(one),
            zero_one(zero),
            zero_two(zero),
            minusone_zero(*gEnvironment, llvm::APInt(32, -1, true)),
            minustwo_two(*gEnvironment, llvm::APInt(32, -2, true)),
            result(*gEnvironment, 32);
    one_two.join(two);
    zero_one.join(one);
    zero_two.join(two);
    minusone_zero.join(zero);
    minustwo_two.join(two);

    llvm::APInt res;

    //Udiv test
    CANAL_ASSERT(result.udiv(one, zero).isTop());
    CANAL_ASSERT(result.udiv(zero, zero).isTop());

    result.udiv(one_two, zero_one);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 1) && //Unsigned one to two
                 result.unsignedMax(res) && res == llvm::APInt(32, 2));
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt::getSignedMinValue(result.getBitWidth()) && //Signed top
                 result.signedMax(res) && res == llvm::APInt::getSignedMaxValue(result.getBitWidth()));

    result.udiv(one_two, minusone_zero); //Division by 0 - maxint => 1 - maxint => 0-2 unsigned
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 2)); //Unsigned zero to two
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt::getSignedMinValue(result.getBitWidth()) && //Signed top
                 result.signedMax(res) && res == llvm::APInt::getSignedMaxValue(result.getBitWidth()));

    result.udiv(one_two, zero_two);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 2));
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt::getSignedMinValue(result.getBitWidth()) && //Signed top
                 result.signedMax(res) && res == llvm::APInt::getSignedMaxValue(result.getBitWidth()));


    //Sdiv test
    CANAL_ASSERT(result.sdiv(one, zero).isTop());
    CANAL_ASSERT(result.sdiv(zero, zero).isTop());

    result.sdiv(one_two, zero_one);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 1) && //Signed one to two
                 result.signedMax(res) && res == llvm::APInt(32, 2));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.sdiv(one_two, minusone_zero); //Division by -1 - 0 -> division by -1
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -2, true) && //Signed minus two to minus one
                 result.signedMax(res) && res == llvm::APInt(32, -1, true));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.sdiv(minustwo_two, minustwo_two); //Division by -2 - 2 -> division by -1 to 1
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -2, true) && //Signed minus two to minus one
                 result.signedMax(res) && res == llvm::APInt(32, 2));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));

    result.sdiv(zero_one, minustwo_two); //Division by -2 - 2 -> division by -1 to 1
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, 1));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getMinValue(result.getBitWidth()) && //Unsigned top
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(result.getBitWidth()));
}

static void
testFPConversions ()
{
    Integer::Interval result(*gEnvironment, llvm::APInt(32, 0)),
            small(*gEnvironment, llvm::APInt(8, 0));
    Float::Interval zerof(*gEnvironment, llvm::APFloat(0.0)),
            pointfivef(*gEnvironment, llvm::APFloat(0.5)),
            zero_pointfivef(zerof),
            negativefive(*gEnvironment, llvm::APFloat(-5.0)),
            minuspointfivef(*gEnvironment, llvm::APFloat(-0.5)),
            five(*gEnvironment, llvm::APFloat(5.0)),
            thousandf(*gEnvironment, llvm::APFloat(1000.0)),
            five_thousandf(five),
            topf(zerof),
            bottomf(zerof),
            smallf(*gEnvironment, llvm::APFloat(1e-10)),
            minusthousand_minusfivef(*gEnvironment, llvm::APFloat(-1000.0)),
            minusthousand_thousandf(minusthousand_minusfivef);
    llvm::APInt res;
    zerof.join(pointfivef);
    five_thousandf.join(thousandf);
    minusthousand_minusfivef.join(negativefive);
    minusthousand_thousandf.join(thousandf);
    topf.setTop();
    bottomf.setBottom();

    //FpToUI
    result.fptoui(zerof);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptoui(pointfivef);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptoui(zero_pointfivef);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptoui(negativefive);
    CANAL_ASSERT(result.isTop());

    result.fptoui(smallf);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptoui(five_thousandf);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 5) &&
                 result.signedMax(res) && res == llvm::APInt(32, 1000));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 5) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 1000));

    result.fptoui(bottomf);
    CANAL_ASSERT(result.isBottom());

    result.fptoui(topf);
    CANAL_ASSERT(result.isTop());

    small.fptoui(thousandf);
    CANAL_ASSERT(small.isTop());

    //FpToSI
    result.fptosi(zerof);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptosi(pointfivef);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptosi(zero_pointfivef);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptosi(minuspointfivef);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptosi(negativefive);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, -5, true) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, -5, true));

    result.fptosi(smallf);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    result.fptosi(five_thousandf);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 5) &&
                 result.signedMax(res) && res == llvm::APInt(32, 1000));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 5) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 1000));

    result.fptosi(minusthousand_minusfivef);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -1000, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, -5, true));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, -1000, true) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, -5, true));

    result.fptosi(minusthousand_thousandf);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -1000, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, 1000));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 1000) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, -1000, true));

    result.fptosi(bottomf);
    CANAL_ASSERT(result.isBottom());

    result.fptosi(topf);
    CANAL_ASSERT(result.isTop());

    small.fptosi(thousandf);
    CANAL_ASSERT(small.isTop());
}

static void
testRemainder() {
    Integer::Interval zero(*gEnvironment, llvm::APInt(32, 0)),
            one(*gEnvironment, llvm::APInt(32, 1)),
            two(*gEnvironment, llvm::APInt(32, 2)),
            four(*gEnvironment, llvm::APInt(32, 4)),
            minusfour(*gEnvironment, llvm::APInt(32, -4, true)),
            ten(*gEnvironment, llvm::APInt(32, 10)),
            minusten(*gEnvironment, llvm::APInt(32, -10, true)),
            five(*gEnvironment, llvm::APInt(32, 5)),
            minusfive(*gEnvironment, llvm::APInt(32, -5, true)),
            five_ten(five),
            minusten_minusfive(minusten),
            one_two(one),
            zero_one(zero),
            zero_two(zero),
            zero_four(zero),
            minusone(*gEnvironment, llvm::APInt(32, -1, true)),
            minustwo(*gEnvironment, llvm::APInt(32, -2, true)),
            minustwo_minusone(minustwo),
            minusone_zero(minusone),
            minustwo_zero(minustwo),
            minustwo_two(minustwo),
            result(*gEnvironment, 32),
            top(zero),
            bottom(zero);
    llvm::APInt res;
    top.setTop();
    bottom.setBottom();
    five_ten.join(ten);
    one_two.join(two);
    zero_one.join(one);
    zero_two.join(two);
    zero_four.join(four);
    minusone_zero.join(zero);
    minustwo_zero.join(zero);
    minustwo_two.join(two);
    minustwo_minusone.join(minusone);
    minusten_minusfive.join(minusfive);

    //urem
    CANAL_ASSERT(result.urem(one, bottom).isBottom());
    CANAL_ASSERT(result.urem(bottom, one).isBottom());
    CANAL_ASSERT(result.urem(one, top).isTop());

    //Constant division
    CANAL_ASSERT(result.urem(one, zero).isTop()); //Division by zero
    CANAL_ASSERT(result.urem(one_two, five).unsignedMin(res) && res == 1 &&
                 result.unsignedMax(res) && res == 2);
    CANAL_ASSERT(result.urem(one_two, two).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 1);
    CANAL_ASSERT(result.urem(zero_one, two).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 1);
    CANAL_ASSERT(result.urem(ten, five).isUnsignedConstant() &&
                 result.unsignedMin(res) && res == 0);
    CANAL_ASSERT(result.urem(five_ten, two).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 1);
    CANAL_ASSERT(result.urem(top, five).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 4);
    CANAL_ASSERT(result.urem(minusone_zero, five).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 4);

    //Interval division
    CANAL_ASSERT(result.urem(one_two, five_ten).unsignedMin(res) && res == 1 &&
                 result.unsignedMax(res) && res == 2);
    CANAL_ASSERT(result.urem(five_ten, one_two).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 1);
    CANAL_ASSERT(result.urem(five_ten, zero_two).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 1);
    CANAL_ASSERT(result.urem(five_ten, zero_one).isUnsignedConstant() &&
                 result.unsignedMin(res) && res == 0);
    CANAL_ASSERT(result.urem(top, five_ten).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 9);
    CANAL_ASSERT(result.urem(minusone_zero, five_ten).unsignedMin(res) && res == 0 &&
                 result.unsignedMax(res) && res == 9);

    //srem
    CANAL_ASSERT(result.srem(one, bottom).isBottom());
    CANAL_ASSERT(result.srem(bottom, one).isBottom());
    CANAL_ASSERT(result.srem(one, top).isTop());

    //Constant division
    CANAL_ASSERT(result.srem(one, zero).isTop()); //Division by zero
    CANAL_ASSERT(result.srem(one_two, five).signedMin(res) && res == 1 &&
                 result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.srem(one_two, two).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(zero_one, two).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(ten, five).isSignedConstant() &&
                 result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.srem(five_ten, two).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(top, minusfive).signedMin(res) && res == llvm::APInt(32, -4, true) &&
                 result.signedMax(res) && res == 4);
    CANAL_ASSERT(result.srem(minusone_zero, five).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);
    CANAL_ASSERT(result.srem(minustwo_two, five).signedMin(res) && res == llvm::APInt(32, -2, true) &&
                 result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.srem(minustwo_two, four).signedMin(res) && res == llvm::APInt(32, -2, true) &&
                 result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.srem(minustwo_two, two).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 1);

    CANAL_ASSERT(result.srem(one_two, minusfive).signedMin(res) && res == 1 &&
                 result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.srem(one_two, minustwo).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(zero_one, minustwo).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(ten, minusfive).isSignedConstant() &&
                 result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.srem(five_ten, minustwo).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(top, minusfive).signedMin(res) && res == llvm::APInt(32, -4, true) &&
                 result.signedMax(res) && res == 4);
    CANAL_ASSERT(result.srem(minusone_zero, minusfive).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);
    CANAL_ASSERT(result.srem(minustwo_two, minusfive).signedMin(res) && res == llvm::APInt(32, -2, true) &&
                 result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.srem(minustwo_two, minusfour).signedMin(res) && res == llvm::APInt(32, -2, true) &&
                 result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.srem(minustwo_two, minustwo).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 1);

    CANAL_ASSERT(result.srem(minustwo_minusone, five).signedMin(res) && res == llvm::APInt(32, -2, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, -1, true));
    CANAL_ASSERT(result.srem(minustwo_minusone, two).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);
    CANAL_ASSERT(result.srem(minusone_zero, two).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);
    CANAL_ASSERT(result.srem(minusten, five).isSignedConstant() &&
                 result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.srem(minusten_minusfive, two).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);

    CANAL_ASSERT(result.srem(minustwo_minusone, minusfive).signedMin(res) && res == llvm::APInt(32, -2, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, -1, true));
    CANAL_ASSERT(result.srem(minustwo_minusone, minustwo).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);
    CANAL_ASSERT(result.srem(minusone_zero, minustwo).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);
    CANAL_ASSERT(result.srem(minusten, minusfive).isSignedConstant() &&
                 result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.srem(minusten_minusfive, minustwo).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);


    //Interval division
    CANAL_ASSERT(result.srem(one_two, five_ten).signedMin(res) && res == 1 &&
                 result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.srem(five_ten, one_two).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(five_ten, zero_two).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(five_ten, zero_one).isSignedConstant() &&
                 result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.srem(top, five_ten).signedMin(res) && res == llvm::APInt(32, -9, true) &&
                 result.signedMax(res) && res == 9);
    CANAL_ASSERT(result.srem(minusone_zero, five_ten).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);

    CANAL_ASSERT(result.srem(one_two, minusten_minusfive).signedMin(res) && res == 1 &&
                 result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.srem(five_ten, minustwo_minusone).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(five_ten, minustwo_zero).signedMin(res) && res == 0 &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(five_ten, minusone_zero).isSignedConstant() &&
                 result.signedMin(res) && res == 0);
    CANAL_ASSERT(result.srem(top, minusten_minusfive).signedMin(res) && res == llvm::APInt(32, -9, true) &&
                 result.signedMax(res) && res == 9);
    CANAL_ASSERT(result.srem(minusone_zero, minusten_minusfive).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 0);

    CANAL_ASSERT(result.srem(minustwo_two, minustwo_two).signedMin(res) && res == llvm::APInt(32, -1, true) &&
                 result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.srem(minustwo_two, zero_four).signedMin(res) && res == llvm::APInt(32, -3, true) &&
                 result.signedMax(res) && res == 3);
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();
    testEquality();
    testJoin();
    testMeet();
    testInclusion();
    testIcmp();
    testTrunc();
    testFPConversions();

    testAdd();
    testDivisionByZero();
    testRemainder();

    delete gEnvironment;
    return 0;
}
