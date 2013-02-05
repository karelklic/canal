#include "lib/IntegerSet.h"
#include "lib/IntegerInterval.h"
#include "lib/FloatInterval.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

using namespace Canal;

static Environment *gEnvironment;

static void
testJoin()
{
    llvm::APInt negone(32, -1, true);
    Integer::Set set1(*gEnvironment, llvm::APInt(32, 0)),
        set2(*gEnvironment, llvm::APInt(32, 1)),
        set3(*gEnvironment, negone),
        set4(set1),
        set5(set2),
        set6(set3),
        result(set1),
        set7(*gEnvironment, llvm::APInt(32, 2)),
        set8(*gEnvironment, llvm::APInt(32, 3)),
        set9(set7),
        set10(set1),
        set11(set1),
            bottom(set1);

    llvm::APInt res;
    bottom.setBottom();

    set4.join(set2); //0-1
    CANAL_ASSERT(set4.signedMin(res) && res == 0 && set4.signedMax(res) && res == 1);

    set4.join(set4); //Join with itself
    CANAL_ASSERT(set4.signedMin(res) && res == 0 && set4.signedMax(res) && res == 1);

    set5.join(set1); //0-1
    CANAL_ASSERT(set5.signedMin(res) && res == 0 && set5.signedMax(res) && res == 1);

    set6.join(set1); //-1-0
    CANAL_ASSERT(set6.signedMin(res) && res == negone && set6.signedMax(res) && res == 0);
    CANAL_ASSERT(set6.unsignedMin(res) && res == 0 && set6.unsignedMax(res) && res == negone);

    set5.join(set6); //-1-1
    CANAL_ASSERT(set5.signedMin(res) && res == negone && set5.signedMax(res) && res == 1);
    CANAL_ASSERT(set5.unsignedMin(res) && res == 0 && set5.unsignedMax(res) && res == negone); //0-(-1)

    set9.join(set8); //2-3
    CANAL_ASSERT(set9.signedMin(res) && res == 2 && set9.signedMax(res) && res == 3);

    set6.join(set8); //-1-3
    CANAL_ASSERT(set6.signedMin(res) && res == negone && set6.signedMax(res) && res == 3);
    CANAL_ASSERT(set6.unsignedMin(res) && res == 0 && set6.unsignedMax(res) && res == negone); //0-(-1)

    set10.setTop(); //Top
    CANAL_ASSERT(set10.isTop());

    set10.join(set10); //Join with itself
    CANAL_ASSERT(set10.isTop());

    set10.join(set4); //Join with any set
    CANAL_ASSERT(set10.isTop());

    //Test of overflow
    for (unsigned i = 0; i < 100000; i ++) {
        set11.join(Integer::Set(*gEnvironment, llvm::APInt(32, i)));
        if (set11.isTop()) break;
    }
    CANAL_ASSERT(set11.isTop());

    CANAL_ASSERT(set2.join(bottom).isConstant() && set2.signedMin(res) == 1);
    CANAL_ASSERT(set10.join(bottom).isTop());
    CANAL_ASSERT(bottom.join(bottom).isBottom());
    CANAL_ASSERT(set4.join(bottom).signedMin(res) && res == 0 &&
                 set4.signedMax(res) && res == 1);
}


static void
testMeet()
{
    Integer::Set
        zero(*gEnvironment, llvm::APInt(32, 0)),
        one(*gEnvironment, llvm::APInt(32, 1)),
        negone(*gEnvironment, llvm::APInt(32, -1, true)),
        bottom(*gEnvironment, 32),
        top(*gEnvironment, 32),
        negone_one(negone),
        zero_one(zero);

    top.setTop();
    negone_one.join(one);
    zero_one.join(one);

    //Top and bottom
    CANAL_ASSERT(bottom.meet(bottom).isBottom());
    CANAL_ASSERT(top.meet(top).isTop());
    {
        Integer::Set result(top), result1(bottom), result2(zero),
                top1(top);
        CANAL_ASSERT(result.meet(bottom).isBottom());
        CANAL_ASSERT(result1.meet(top).isBottom());
        CANAL_ASSERT(result2.meet(top) == zero);
        CANAL_ASSERT(top1.meet(zero) == zero);
        CANAL_ASSERT(result2.meet(bottom).isBottom());
    }

    //Tests with values
    {
        Integer::Set zero1(zero), one1(one), negone1(negone),
                negone_one1(negone_one), negone_one2(negone_one), zero_one1(zero_one);
        CANAL_ASSERT(zero1.meet(zero) == zero);
        CANAL_ASSERT(one1.meet(one) == one);
        CANAL_ASSERT(negone1.meet(negone) == negone);
        CANAL_ASSERT(negone_one1.meet(negone_one) == negone_one);
        CANAL_ASSERT(zero_one1.meet(zero_one) == zero_one);

        CANAL_ASSERT(zero1.meet(one).isBottom());
        CANAL_ASSERT(one1.meet(zero_one) == one);
        CANAL_ASSERT(one1.meet(negone_one) == one);

        CANAL_ASSERT(negone_one1.meet(one) == one);
        CANAL_ASSERT(negone_one2.meet(zero_one) == one);
        CANAL_ASSERT(zero_one1.meet(negone_one) == negone_one2);

        CANAL_ASSERT(negone1.meet(negone_one) == negone);
    }
}

static void
testInclusion()
{
    Integer::Set
        zero(*gEnvironment, llvm::APInt(32, 0)),
        one(*gEnvironment, llvm::APInt(32, 1)),
        negone(*gEnvironment, llvm::APInt(32, -1, true)),
        bottom(*gEnvironment, 32),
        top(*gEnvironment, 32),
        negone_one(negone),
        zero_one(zero);

    top.setTop();
    negone_one.join(one);
    zero_one.join(one);

    //Top and bottom
    CANAL_ASSERT(bottom < bottom);
    CANAL_ASSERT(top < top);
    CANAL_ASSERT(!(top < bottom));
    CANAL_ASSERT(!(zero < bottom));
    CANAL_ASSERT(bottom < zero);
    CANAL_ASSERT(zero < top);
    CANAL_ASSERT(!(top < zero));

    //Tests with values
    CANAL_ASSERT(zero < zero);
    CANAL_ASSERT(one < one);
    CANAL_ASSERT(negone < negone);
    CANAL_ASSERT(negone_one < negone_one);
    CANAL_ASSERT(zero_one < zero_one);

    CANAL_ASSERT(!(zero < one));
    CANAL_ASSERT(one < zero_one);
    CANAL_ASSERT(!(zero_one < one));
    CANAL_ASSERT(one < negone_one);
    CANAL_ASSERT(!(negone_one < one));

    CANAL_ASSERT(!(negone_one < zero_one));
    CANAL_ASSERT(!(zero_one < negone_one));

    CANAL_ASSERT(negone < negone_one);
    CANAL_ASSERT(!(negone_one < negone));
}

static void
testIcmp()
{
    Integer::Set set1(*gEnvironment, llvm::APInt(32, 0)),
        set2(*gEnvironment, llvm::APInt(32, 1)),
        set3(*gEnvironment, llvm::APInt(32, -1, true)),
        set4(set1),
        set5(set2),
        set6(set3),
        result(set1),
        set7(*gEnvironment, llvm::APInt(32, 2)),
        set8(*gEnvironment, llvm::APInt(32, 3)),
        set9(set7),
        set10(set1),
        set11(set2);

    llvm::APInt res;
    set4.join(set2); //0-1
    set5.join(set1); //0-1
    set6.join(set1); //-1-0
    set9.join(set8); //2-3
    set10.join(set7); //0-2
    set11.join(set8); //1-3

    result.icmp(set1, set2, llvm::CmpInst::ICMP_EQ); //0 != 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_NE); //0 != 1
    CANAL_ASSERT(result.isTrue());

    //Difference from interval
    result.icmp(set2, set6, llvm::CmpInst::ICMP_EQ); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> false
    CANAL_ASSERT(result.isFalse()); //False


    result.icmp(set2, set6, llvm::CmpInst::ICMP_NE); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> true
    CANAL_ASSERT(result.isTrue()); //True

    result.icmp(set4, set7, llvm::CmpInst::ICMP_EQ); //0-1 != 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_NE); //0-1 != 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_EQ); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_NE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_EQ); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_NE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_EQ); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_NE); //0-1 ==  0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_EQ); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_NE); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(set1, set4, llvm::CmpInst::ICMP_EQ); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set1, set4, llvm::CmpInst::ICMP_NE); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_EQ); //0-1 != 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_NE); //0-1 != 2-3
    CANAL_ASSERT(result.isTrue());

    //Difference from interval
    result.icmp(set10, set11, llvm::CmpInst::ICMP_EQ); //0-2 != 1-3 (no intersection)
    CANAL_ASSERT(result.isFalse());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_NE); //0-2 != 1-3 (no intersection)
    CANAL_ASSERT(result.isTrue());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_SGT); //0 > 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_SGE); //0 > 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_UGT); //0 > 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_UGE); //0 > 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_SLT); //0 > 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_SLE); //0 > 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_ULT); //0 > 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(set1, set2, llvm::CmpInst::ICMP_ULE); //0 > 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(set2, set6, llvm::CmpInst::ICMP_SLE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isFalse());

    result.icmp(set2, set6, llvm::CmpInst::ICMP_SLT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isFalse());

    result.icmp(set2, set6, llvm::CmpInst::ICMP_ULE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(set2, set6, llvm::CmpInst::ICMP_ULT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(set2, set6, llvm::CmpInst::ICMP_SGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTrue());

    result.icmp(set2, set6, llvm::CmpInst::ICMP_SGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTrue());

    result.icmp(set2, set6, llvm::CmpInst::ICMP_UGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(set2, set6, llvm::CmpInst::ICMP_UGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(set1, set6, llvm::CmpInst::ICMP_SLE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isFalse());

    result.icmp(set1, set6, llvm::CmpInst::ICMP_SLT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(set1, set6, llvm::CmpInst::ICMP_ULE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTrue());

    result.icmp(set1, set6, llvm::CmpInst::ICMP_ULT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(set1, set6, llvm::CmpInst::ICMP_SGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTrue());

    result.icmp(set1, set6, llvm::CmpInst::ICMP_SGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(set1, set6, llvm::CmpInst::ICMP_UGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isFalse());

    result.icmp(set1, set6, llvm::CmpInst::ICMP_UGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_SGE); //0-1 < 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_SGT); //0-1 < 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_UGE); //0-1 < 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_UGT); //0-1 < 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_SLE); //0-1 < 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_SLT); //0-1 < 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_ULE); //0-1 < 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set7, llvm::CmpInst::ICMP_ULT); //0-1 < 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_SGE); //0-1 <= 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_SGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_UGE); //0-1 <= 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_UGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_SLE); //0-1 <= 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_SLT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_ULE); //0-1 <= 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set1, llvm::CmpInst::ICMP_ULT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_UGT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_UGE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_SGT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_SGE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_ULT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_ULE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_SLT); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set5, llvm::CmpInst::ICMP_SLE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_UGT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_UGE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_SGT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_SGE); //0-1 <= -1-0 //Signed
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_ULT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_ULE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_SLT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set6, llvm::CmpInst::ICMP_SLE); //0-1 <= -1-0 //Signed
    CANAL_ASSERT(result.isFalse());


    result.icmp(set4, set4, llvm::CmpInst::ICMP_SGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_SGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_UGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_UGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_SLT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_SLE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_ULT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(set4, set4, llvm::CmpInst::ICMP_ULE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());


    result.icmp(set4, set9, llvm::CmpInst::ICMP_SGE); //0-1 < 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_SGT); //0-1 < 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_UGE); //0-1 < 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_UGT); //0-1 < 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_SLE); //0-1 < 2-3
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_SLT); //0-1 < 2-3
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_ULE); //0-1 < 2-3
    CANAL_ASSERT(result.isTrue());

    result.icmp(set4, set9, llvm::CmpInst::ICMP_ULT); //0-1 < 2-3
    CANAL_ASSERT(result.isTrue());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_SGE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_SGT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_UGE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_UGT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_SLE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_SLT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_ULE); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    result.icmp(set10, set11, llvm::CmpInst::ICMP_ULT); //0-2 ? 1-3
    CANAL_ASSERT(result.isTop());

    Integer::Set bottom(set1), top(set1);
    bottom.setBottom();
    top.setTop();
    CANAL_ASSERT(result.icmp(bottom, bottom, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(top, bottom, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(set1, bottom, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(bottom, top, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(bottom, set1, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(top, top, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(top, set1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(set1, top, llvm::CmpInst::ICMP_EQ).isTop());
}

static void
testDivisionByZero() {
    Integer::Set zero(*gEnvironment, llvm::APInt(32, 0)),
            one(*gEnvironment, llvm::APInt(32, 1)),
            two(*gEnvironment, llvm::APInt(32, 2)),
            one_two(one),
            zero_one(zero),
            minusone_zero(*gEnvironment, llvm::APInt(32, -1, true)),
            minusone_minustwo(*gEnvironment, llvm::APInt(32, -2, true)),
            result(*gEnvironment, 32);
    one_two.join(two);
    zero_one.join(one);
    minusone_minustwo.join(minusone_zero); //minusone_zero is now only -1
    minusone_zero.join(zero);

    //Udiv test
    CANAL_ASSERT(result.udiv(one, zero).isTop());
    CANAL_ASSERT(result.udiv(zero, zero).isTop());
    CANAL_ASSERT(result.udiv(one_two, zero_one) == one_two);
    CANAL_ASSERT(result.udiv(one_two, minusone_zero) == zero); //Division by 0 - maxint => zero

    //Sdiv test
    CANAL_ASSERT(result.sdiv(one, zero).isTop());
    CANAL_ASSERT(result.sdiv(zero, zero).isTop());
    CANAL_ASSERT(result.sdiv(one_two, zero_one) == one_two);
    CANAL_ASSERT(result.sdiv(one_two, minusone_zero) == minusone_minustwo);

    //Urem test
    CANAL_ASSERT(result.urem(one, zero).isTop());
    CANAL_ASSERT(result.urem(zero, zero).isTop());
    CANAL_ASSERT(result.urem(one_two, zero_one) == zero);
    CANAL_ASSERT(result.urem(one_two, minusone_zero) == one_two); //Remainder of 0 - maxint => identity

    //Srem test
    CANAL_ASSERT(result.srem(one, zero).isTop());
    CANAL_ASSERT(result.srem(zero, zero).isTop());
    CANAL_ASSERT(result.srem(one_two, zero_one) == zero);
    CANAL_ASSERT(result.srem(one_two, minusone_zero) == zero);
}

static void
testAdd()
{
    Integer::Set zero(*gEnvironment, llvm::APInt(32, 0)),
            one(*gEnvironment, llvm::APInt(32, 1)),
            minusone(*gEnvironment, llvm::APInt(32, -1, true)),
            two(*gEnvironment, llvm::APInt(32, 2)),
            zero_one(zero), zero_one2(one), minusone_zero(minusone), result(zero), two_three(two),
            three(*gEnvironment, llvm::APInt(32, 3)),
            one_three(one), top(zero), bottom(zero);
    llvm::APInt res;

    zero_one.join(one); //0-1
    zero_one2.join(zero); //0-1
    minusone_zero.join(zero); //-1-0
    two_three.join(three); //2-3
    one_three.join(three); //1-3
    top.setTop();
    bottom.setBottom();

    result.add(zero, zero); //0 + 0 = 0
    CANAL_ASSERT(result == zero);

    result.add(zero, top); //0 + TOP = TOP
    CANAL_ASSERT(result.isTop());

    result.add(zero_one, zero); //0-1 + 0 = 0-1
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == 1);

    result.add(zero_one, zero_one2); //0-1 + 0-1 = 0-2
    CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 2);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == 2);

    result.add(two_three, one_three); //2-3 + 1-3 = 3-6
    CANAL_ASSERT(result.signedMin(res) && res == 3 && result.signedMax(res) && res == 6);
    CANAL_ASSERT(result.unsignedMin(res) && res == 3 && result.unsignedMax(res) && res == 6);

    result.add(zero_one, minusone_zero); //0-1 + -1-0 = -1-1 / TOP for unsigned
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -1, true) && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == llvm::APInt(32, -1, true));

    Integer::Set tmp1(result);
    result.add(tmp1, minusone_zero); //-1-1 + -1-0 = -2-1 / TOP for unsigned
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -2, true) && result.signedMax(res) && res == 1);
    CANAL_ASSERT(result.unsignedMin(res) && res == 0 && result.unsignedMax(res) && res == llvm::APInt(32, -1, true));

    result.add(top, zero); //TOP + anything = TOP
    CANAL_ASSERT(result.isTop());

    result.add(top, zero_one2); //TOP + anything = TOP
    CANAL_ASSERT(result.isTop());

    for (unsigned i = 0; i < 30; i ++) {
        Integer::Set tmp(i ? result : two_three);
        result.add(tmp, two_three); //2-3
    }
    CANAL_ASSERT(result.signedMin(res) && res == 2*31 && result.signedMax(res) && res == 3*31);
    CANAL_ASSERT(result.unsignedMin(res) && res == 2*31 && result.unsignedMax(res) && res == 3*31);

    for (unsigned i = 0; i < 30; i ++) {
        Integer::Set tmp(result);
        result.add(tmp, minusone_zero);
    }
    //CANAL_ASSERT(result.signedMin(res) && res == 0 && result.signedMax(res) && res == 3000);
    CANAL_ASSERT(result.isTop());

    //Bottom values
    CANAL_ASSERT(result.add(bottom, top).isBottom());
    CANAL_ASSERT(result.add(top, bottom).isBottom());
    CANAL_ASSERT(result.add(bottom, zero).isBottom());
    CANAL_ASSERT(result.add(zero, bottom).isBottom());
    CANAL_ASSERT(result.add(bottom, bottom).isBottom());
}

static void
testIntervalConversion () {
    Integer::Interval zero(*gEnvironment, llvm::APInt(32, 0)),
            one(*gEnvironment, llvm::APInt(32, 1)),
            two(*gEnvironment, llvm::APInt(32, 2)),
            one_two(one),
            zero_one(zero),
            zero_two(zero),
            minusone_zero(*gEnvironment, llvm::APInt(32, -1, true)),
            minustwo_two(*gEnvironment, llvm::APInt(32, -2, true)),
            thousand(*gEnvironment, llvm::APInt(32, 1000)),
            zero_thousand(zero),
            bottom(zero),
            top(zero),
            signedTop(zero),
            unsignedTop(zero),
            signedBottom(zero),
            unsignedBottom(zero),
            signedOverflow(*gEnvironment, llvm::APInt(32, 2147483647)),
            signedOverflow2(*gEnvironment, llvm::APInt::getSignedMinValue(32));
    Integer::Set result(*gEnvironment, 32);
    llvm::APInt res;
    one_two.join(two);
    zero_one.join(one);
    zero_two.join(two);
    minusone_zero.join(zero);
    minustwo_two.join(two);
    zero_thousand.join(thousand);
    bottom.setBottom();
    top.setTop();
    signedTop.setSignedTop();
    unsignedTop.setUnsignedTop();
    signedBottom.setSignedBottom();
    unsignedBottom.setUnsignedBottom();
    signedOverflow.join(signedOverflow2); //Two values only when sorted by

    CANAL_ASSERT(result.fromInterval(bottom).isBottom());
    CANAL_ASSERT(result.fromInterval(top).isTop());
    CANAL_ASSERT(result.fromInterval(signedTop).isTop());
    CANAL_ASSERT(result.fromInterval(unsignedTop).isTop());
    CANAL_ASSERT(result.fromInterval(signedBottom).isBottom());
    CANAL_ASSERT(result.fromInterval(unsignedBottom).isBottom());

    CANAL_ASSERT(result.fromInterval(zero).isConstant() &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));
    CANAL_ASSERT(result.fromInterval(one).isConstant() &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 1));
    CANAL_ASSERT(result.fromInterval(two).isConstant() &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 2));

    result.fromInterval(one_two);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 1) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 2));
    result.fromInterval(zero_one);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 1));
    result.fromInterval(zero_two);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 2));

    result.fromInterval(minusone_zero);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, -1, true));

    result.fromInterval(minustwo_two); //-2, -1, 0, 1, 2
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt::getMaxValue(32) &&
                 result.signedMin(res) && res == llvm::APInt(32, -2, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, 2));

    CANAL_ASSERT(result.fromInterval(zero_thousand).isTop()); //Too many values

    result.fromInterval(signedOverflow); //Signed overflow, but not unsigned
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 2147483647) &&
                 result.unsignedMax(res) && res == llvm::APInt::getSignedMinValue(32));
}

static void
testFPConversions ()
{
    Integer::Set result(*gEnvironment, llvm::APInt(32, 0)),
            small(*gEnvironment, llvm::APInt(8, 0));
    Float::Interval zerof(*gEnvironment, llvm::APFloat(0.0)),
            pointfivef(*gEnvironment, llvm::APFloat(0.5)),
            zero_pointfivef(zerof),
            minusfivef(*gEnvironment, llvm::APFloat(-5.0)),
            minuspointfivef(*gEnvironment, llvm::APFloat(-0.5)),
            fivef(*gEnvironment, llvm::APFloat(5.0)),
            thirtyf(*gEnvironment, llvm::APFloat(30.0)),
            five_thirtyf(fivef),
            thousandf(*gEnvironment, llvm::APFloat(1000.0)),
            five_thousandf(fivef),
            topf(zerof),
            bottomf(zerof),
            smallf(*gEnvironment, llvm::APFloat(1e-10)),
            minusthousand_minusfivef(*gEnvironment, llvm::APFloat(-1000.0)),
            minusthousand_thousandf(minusthousand_minusfivef),
            minusten_minusfivef(*gEnvironment, llvm::APFloat(-10.0)),
            minusten_tenf(*gEnvironment, llvm::APFloat(10.0));
    llvm::APInt res;
    zerof.join(pointfivef);
    five_thousandf.join(thousandf);
    five_thirtyf.join(thirtyf);
    minusthousand_minusfivef.join(minusfivef);
    minusthousand_thousandf.join(thousandf);
    minusten_tenf.join(minusten_minusfivef);
    minusten_minusfivef.join(minusfivef);
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

    result.fptoui(minusfivef);
    CANAL_ASSERT(result.isTop());

    result.fptoui(smallf);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    CANAL_ASSERT(result.fptoui(five_thousandf).isTop()); //Too many numbers -> TOP

    result.fptoui(five_thirtyf);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 5) &&
                 result.signedMax(res) && res == llvm::APInt(32, 30));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 5) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 30));

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

    result.fptosi(minusfivef);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, -5, true) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, -5, true));

    result.fptosi(smallf);
    CANAL_ASSERT(result.isConstant() &&
                 result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMin(res) && res == llvm::APInt(32, 0));

    CANAL_ASSERT(result.fptosi(five_thousandf).isTop()); //Too many numbers -> TOP

    result.fptosi(five_thirtyf);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 5) &&
                 result.signedMax(res) && res == llvm::APInt(32, 30));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 5) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 30));

    CANAL_ASSERT(result.fptosi(minusthousand_minusfivef).isTop());  //Too many numbers -> TOP

    result.fptosi(minusten_minusfivef);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -10, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, -5, true));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, -10, true) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, -5, true));

    CANAL_ASSERT(result.fptosi(minusthousand_thousandf).isTop()); //Too many numbers -> TOP

    result.fptosi(minusten_tenf); //-10, -9, -8... -1, 0, 1... 10 -> unsigned order 0, 1, ... 10, -10... -2, -1
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -10, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, 10));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, -1, true));

    result.fptosi(bottomf);
    CANAL_ASSERT(result.isBottom());

    result.fptosi(topf);
    CANAL_ASSERT(result.isTop());

    small.fptosi(thousandf);
    CANAL_ASSERT(small.isTop());
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testJoin();
    testMeet();
    testInclusion();
    testIcmp();
    testDivisionByZero();

    testAdd();

    testIntervalConversion();
    testFPConversions();


    delete gEnvironment;
    return 0;
}
