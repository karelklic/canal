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
    testIcmp();
    testTrunc();

    delete gEnvironment;
    return 0;
}
