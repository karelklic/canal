#include "lib/IntegerBitfield.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>
#include "lib/IntegerInterval.h"
#include "lib/FloatInterval.h"

using namespace Canal;

static Environment *gEnvironment;

static void
testJoin()
{
    Integer::Bitfield bitfield1(*gEnvironment, llvm::APInt(32, 0)),
        bitfield2(*gEnvironment, llvm::APInt(32, 1)),
        bitfield3(*gEnvironment, llvm::APInt(32, -1, true)),
        bitfield4(bitfield1),
        bitfield5(bitfield2),
        bitfield6(bitfield3),
        result(bitfield1),
        bitfield7(*gEnvironment, llvm::APInt(32, 2)),
        bitfield8(*gEnvironment, llvm::APInt(32, 3)),
        bitfield9(bitfield7),
        bitfield10(bitfield1),
        bitfield11(bitfield2),
            bottom(bitfield1);

    llvm::APInt res;
    bottom.setBottom();

    bitfield4.join(bitfield2); //0-1 -> ...00T
    CANAL_ASSERT(bitfield4.unsignedMin(res) && res == 0 && bitfield4.unsignedMax(res) && res == 1);
    CANAL_ASSERT(bitfield4.signedMin(res) && res == 0 && bitfield4.signedMax(res) && res == 1);

    bitfield4.join(bitfield4); //Join with itself
    CANAL_ASSERT(bitfield4.unsignedMin(res) && res == 0 && bitfield4.unsignedMax(res) && res == 1);
    CANAL_ASSERT(bitfield4.signedMin(res) && res == 0 && bitfield4.signedMax(res) && res == 1);

    bitfield5.join(bitfield1); //0-1 -> ...00T
    CANAL_ASSERT(bitfield5.unsignedMin(res) && res == 0 && bitfield5.unsignedMax(res) && res == 1);
    CANAL_ASSERT(bitfield5.signedMin(res) && res == 0 && bitfield5.signedMax(res) && res == 1);

    bitfield6.join(bitfield1); //-1-0 -> TOP
    CANAL_ASSERT(bitfield6.isTop());
    CANAL_ASSERT(bitfield6.signedMin(res) && res == llvm::APInt::getSignedMinValue(8*sizeof(int))
                 && bitfield6.signedMax(res) && res == llvm::APInt::getSignedMaxValue(8*sizeof(int)));
    CANAL_ASSERT(bitfield6.unsignedMin(res) && res == 0
                 && bitfield6.unsignedMax(res) && res == llvm::APInt::getMaxValue(8*sizeof(int)));

    bitfield9.join(bitfield8); //2-3 -> ...01T
    CANAL_ASSERT(bitfield9.unsignedMin(res) && res == 2 && bitfield9.unsignedMax(res) && res == 3);
    CANAL_ASSERT(bitfield9.signedMin(res) && res == 2 && bitfield9.signedMax(res) && res == 3);

    bitfield5.join(bitfield8); //0-3 -> ...0TT
    CANAL_ASSERT(bitfield5.signedMin(res) && res == 0 && bitfield5.signedMax(res) && res == 3);
    CANAL_ASSERT(bitfield5.unsignedMin(res) && res == 0 && bitfield5.unsignedMax(res) && res == 3);

    bitfield10.setTop(); //Top
    CANAL_ASSERT(bitfield10.isTop());

    bitfield10.join(bitfield10); //Join with itself
    CANAL_ASSERT(bitfield10.isTop());

    bitfield10.join(bitfield4); //Join with any bitfield
    CANAL_ASSERT(bitfield10.isTop());

    for (unsigned i = 0; i < 1024; i ++)
        bitfield11.join(Integer::Bitfield(*gEnvironment, llvm::APInt(32, i)));

    //0-1023 -> ...0TTTTTTTTTT (last ten bits is T)
    CANAL_ASSERT(bitfield11.signedMin(res) && res == 0 && bitfield11.signedMax(res) && res == 1023);
    CANAL_ASSERT(bitfield11.unsignedMin(res) && res == 0 && bitfield11.unsignedMax(res) && res == 1023);

    //Bottom values
    CANAL_ASSERT(bitfield2.join(bottom).isConstant() && bitfield2.signedMin(res) == 1);
    CANAL_ASSERT(bitfield10.join(bottom).isTop());
    CANAL_ASSERT(bottom.join(bottom).isBottom());
    CANAL_ASSERT(bitfield4.join(bottom).signedMin(res) && res == 0 &&
                 bitfield4.signedMax(res) && res == 1);
}

static void
testMeet()
{
    Integer::Bitfield
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
        Integer::Bitfield result(top), result1(bottom), result2(zero),
                top1(top);
        CANAL_ASSERT(result.meet(bottom).isBottom());
        CANAL_ASSERT(result1.meet(top).isBottom());
        CANAL_ASSERT(result2.meet(top) == zero);
        CANAL_ASSERT(top1.meet(zero) == zero);
        CANAL_ASSERT(result2.meet(bottom).isBottom());
    }

    //Tests with values
    {
        Integer::Bitfield zero1(zero), one1(one), negone1(negone),
                negone_one1(negone_one), negone_one2(negone_one), zero_one1(zero_one);
        CANAL_ASSERT(zero1.meet(zero) == zero);
        CANAL_ASSERT(one1.meet(one) == one);
        CANAL_ASSERT(negone1.meet(negone) == negone);
        CANAL_ASSERT(negone_one1.meet(negone_one) == negone_one);
        CANAL_ASSERT(zero_one1.meet(zero_one) == zero_one);

        CANAL_ASSERT(zero1.meet(one).toString() == "integerBitfield 0000000000000000000000000000000_\n");
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
    Integer::Bitfield
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
    Integer::Bitfield const0(*gEnvironment, llvm::APInt(32, 0)),
        const1(*gEnvironment, llvm::APInt(32, 1)),
        const_1(*gEnvironment, llvm::APInt(32, -1, true)),
        const0to1(const0),
        const0to1_2(const1),
        const_1to0(const_1),
        result(*gEnvironment, llvm::APInt(1, 1)),
        const2(*gEnvironment, llvm::APInt(32, 2)),
        const3(*gEnvironment, llvm::APInt(32, 3)),
        const2to3(const2),
        const0to2(const0),
        const1to3(const1);

    const0to1.join(const1); //0-1
    const0to1_2.join(const0); //0-1
    const_1to0.join(const0); //-1-0
    const2to3.join(const3); //2-3
    const0to2.join(const2); //0-2
    const1to3.join(const3); //1-3

    //0 = 0
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

    //0 < 1
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

    //1 > 0
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_SGT).isTrue());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_UGT).isTrue());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_UGE).isTrue());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_SLE).isFalse());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_ULT).isFalse());
    CANAL_ASSERT(result.icmp(const1, const0, llvm::CmpInst::ICMP_ULE).isFalse());

    //Difference from interval
    //1 >= 0-1 (when unsigned, -1-0 is 0-max)
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_ULE).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const1, const_1to0, llvm::CmpInst::ICMP_UGT).isTop());

    //Difference from interval
     //0 ? -1-0 (when unsigned, -1-0 is 0-max)
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_ULE).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const0, const_1to0, llvm::CmpInst::ICMP_UGT).isTop());

    //0-1 < 2
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_SGE).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_UGE).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_SLT).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_ULE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const2, llvm::CmpInst::ICMP_ULT).isTrue());

    //0-1 >= 0
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_UGE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_ULE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_ULT).isFalse());

    //0 <= 0-1
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(const0, const0to1, llvm::CmpInst::ICMP_ULE).isTrue());

    //0-1 >= 0
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_UGE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_ULT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const0, llvm::CmpInst::ICMP_ULE).isTop());

    //0-1 <= 1
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_ULE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const1, llvm::CmpInst::ICMP_ULT).isTop());

    //0-1 ? 0-1
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_ULE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1_2, llvm::CmpInst::ICMP_SLE).isTop());

    //0-1 ? 0-1 (same object)
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const0to1, llvm::CmpInst::ICMP_ULE).isTop());

    //Difference from interval
    //0-1 ? -1-0
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_ULE).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const0to1, const_1to0, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.isTop());

    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_SGE).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_UGE).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_SLT).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_ULE).isTrue());
    CANAL_ASSERT(result.icmp(const0to1, const2to3, llvm::CmpInst::ICMP_ULT).isTrue());

    //Difference from interval
    //0-2 != 1-3 (difference in last bit), but 0-2 ? 1-3 in inequality comparison
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_ULE).isTop());
    CANAL_ASSERT(result.icmp(const0to2, const1to3, llvm::CmpInst::ICMP_ULT).isTop());

    //Tests for bitwidth = 1
    Integer::Bitfield width1const0(*gEnvironment, llvm::APInt(1, 0)), //0
            width1const1(*gEnvironment, llvm::APInt(1, 1)), //1 unsigned, -1 signed
            width1const0to1(*gEnvironment, llvm::APInt(1, 0)); //TOP
    width1const0to1.join(width1const1);

    //0 = 0
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_EQ).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_NE).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_UGE).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_ULT).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const0, llvm::CmpInst::ICMP_ULE).isTrue());

    //0 < 1 (signed: 0 > -1)
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_SGT).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_UGE).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_SLE).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_ULT).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const1, llvm::CmpInst::ICMP_ULE).isTrue());

    //1 > 0 (signed: -1 < 0)
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_SGE).isFalse());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_UGT).isTrue());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_UGE).isTrue());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_SLT).isTrue());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_ULT).isFalse());
    CANAL_ASSERT(result.icmp(width1const1, width1const0, llvm::CmpInst::ICMP_ULE).isFalse());

    //0 <= 0-1 (signed: 0 >= -1-0)
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_ULT).isTop());
    CANAL_ASSERT(result.icmp(width1const0, width1const0to1, llvm::CmpInst::ICMP_ULE).isTrue());

    //T ? T
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_SGT).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_UGT).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_SLE).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_ULE).isTop());
    CANAL_ASSERT(result.icmp(width1const0to1, width1const0to1, llvm::CmpInst::ICMP_ULT).isTop());

    //Signed tests
    Integer::Bitfield const_10(*gEnvironment, llvm::APInt(32, -10, true)),
            const_5(*gEnvironment, llvm::APInt(32, -5, true)),
            const_2_1(*gEnvironment, llvm::APInt(32, -2, true));
    const_2_1.join(const_1);
    //-10 < -5
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_SGE).isFalse());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_UGE).isFalse());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_SLT).isTrue());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_ULT).isTrue());
    CANAL_ASSERT(result.icmp(const_10, const_5, llvm::CmpInst::ICMP_ULE).isTrue());

    //-5 > -10
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_EQ).isFalse());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_NE).isTrue());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_SGT).isTrue());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_SGE).isTrue());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_UGT).isTrue());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_UGE).isTrue());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_SLT).isFalse());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_SLE).isFalse());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_ULT).isFalse());
    CANAL_ASSERT(result.icmp(const_5, const_10, llvm::CmpInst::ICMP_ULE).isFalse());

    //-2-1 <= -1
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_NE).isTop());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_SGE).isTop());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_SGT).isFalse());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_UGE).isTop());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_UGT).isFalse());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_SLE).isTrue());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_SLT).isTop());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_ULE).isTrue());
    CANAL_ASSERT(result.icmp(const_2_1, const_1, llvm::CmpInst::ICMP_ULT).isTop());


    //Bottom tests
    Integer::Bitfield bottom(const0), top(const0);
    bottom.setBottom();
    top.setTop();
    CANAL_ASSERT(result.icmp(bottom, bottom, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(top, bottom, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(const0, bottom, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(bottom, top, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(bottom, const0, llvm::CmpInst::ICMP_EQ).isBottom());
    CANAL_ASSERT(result.icmp(top, top, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(top, const0, llvm::CmpInst::ICMP_EQ).isTop());
    CANAL_ASSERT(result.icmp(const0, top, llvm::CmpInst::ICMP_EQ).isTop());
}

static void
testShl () {
    Integer::Bitfield one(*gEnvironment, llvm::APInt(32, 1)),
            bitsize(*gEnvironment, llvm::APInt(32, 32)),
            zero(*gEnvironment, llvm::APInt(32, 0)),
            two(*gEnvironment, llvm::APInt(32, 2)),
            zero_one(zero),
            three(*gEnvironment, llvm::APInt(32, 3)),
            one_three(one),
            bottom(zero), top(zero);
    zero_one.join(one);
    one_three.join(three);
    llvm::APInt res;
    top.setTop();
    bottom.setBottom();

    {
        Integer::Bitfield result(zero);
        CANAL_ASSERT(result.shl(bottom, bottom).isBottom());
        CANAL_ASSERT(result.shl(bottom, top).isBottom());
        CANAL_ASSERT(result.shl(top, bottom).isBottom());
        CANAL_ASSERT(result.shl(bottom, zero).isBottom());
        CANAL_ASSERT(result.shl(zero, bottom).isBottom());

        CANAL_ASSERT(result.shl(top, top).isTop());
        CANAL_ASSERT(result.shl(zero, top).isTop());
        CANAL_ASSERT(result.shl(top, zero).isTop());
    }

    { //Shift left for more than bitsize -> 0
        Integer::Bitfield tmp(one);
        tmp.shl(one, bitsize);
        CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == 0);
    }

    { //Identity
        Integer::Bitfield tmp(zero);
        CANAL_ASSERT(tmp.shl(three, zero) == three);
    }

    { //Shift one to the left, one bit per iteration
        Integer::Bitfield tmp(one);
        for (unsigned i = 1; i < 31; i ++) { //Basic left shift
            Integer::Bitfield t(tmp);
            tmp.shl(t, one);
            CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == pow(2, i));
        }
    }

    { //Shift one to the left by zero to one -> merge of
        //...0001
        //...0010 ->
        //...00TT
        Integer::Bitfield result(zero), tmp2(zero);
        result.shl(one, zero_one);
        CANAL_ASSERT(result.unsignedMin(res) && res == 0 &&
                     result.unsignedMax(res) && res == 3);
        tmp2.shl(result, zero_one); //Move ...00TT to left by zero_one -> 0TTT
        CANAL_ASSERT(tmp2.unsignedMin(res) && res == 0 &&
                     tmp2.unsignedMax(res) && res == 7);
        tmp2.shl(one, one_three); //Move ...0001 to left by one_three -> ...0000TTT0
        CANAL_ASSERT(tmp2.unsignedMin(res) && res == 0 &&
                     tmp2.unsignedMax(res) && res == 14);
    }

    { //Shift left of 01010101...0101
        Integer::Bitfield val(*gEnvironment, llvm::APInt(32, 1431655765)), tmp(zero);
        CANAL_ASSERT(tmp.shl(val, zero_one).isTop()); //Move to left by zero to one -> top
        tmp.shl(val, two);
        CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == 1431655764); //Move to left by two -> 01010101...0100
    }
}

static void
testLshr () {
    Integer::Bitfield one(*gEnvironment, llvm::APInt(32, 1)),
            one_shift(*gEnvironment, llvm::APInt::getSignedMinValue(32)), //10000...000
            bitsize(*gEnvironment, llvm::APInt(32, 32)),
            zero(*gEnvironment, llvm::APInt(32, 0)),
            two(*gEnvironment, llvm::APInt(32, 2)),
            zero_one(zero),
            three(*gEnvironment, llvm::APInt(32, 3)),
            one_three(one),
            bottom(zero), top(zero);
    zero_one.join(one);
    one_three.join(three);
    llvm::APInt res;
    top.setTop();
    bottom.setBottom();

    {
        Integer::Bitfield result(zero);
        CANAL_ASSERT(result.shl(bottom, bottom).isBottom());
        CANAL_ASSERT(result.shl(bottom, top).isBottom());
        CANAL_ASSERT(result.shl(top, bottom).isBottom());
        CANAL_ASSERT(result.shl(bottom, zero).isBottom());
        CANAL_ASSERT(result.shl(zero, bottom).isBottom());

        CANAL_ASSERT(result.shl(top, top).isTop());
        CANAL_ASSERT(result.shl(zero, top).isTop());
        CANAL_ASSERT(result.shl(top, zero).isTop());
    }

    { //Shift rigth for more than bitsize -> 0
        Integer::Bitfield tmp(one);
        tmp.lshr(one_shift, bitsize);
        CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == 0);
    }

    { //Identity
        Integer::Bitfield tmp(zero);
        CANAL_ASSERT(tmp.lshr(three, zero) == three);
    }

    { //Shift one to the right, one bit per iteration
        Integer::Bitfield tmp(one_shift);
        for (unsigned i = 1; i < 31; i ++) { //Basic right shift
            Integer::Bitfield t(tmp);
            tmp.lshr(t, one);
            CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == pow(2, 31 - i));
        }
    }

    { //Shift one to the right by zero to one -> merge of
        //1000...0
        //0100...0 ->
        //TT00...0
        Integer::Bitfield result(zero), tmp2(zero);
        result.lshr(one_shift, zero_one);
        CANAL_ASSERT(result.unsignedMin(res) && res == 0 &&
                     result.unsignedMax(res) && res == 3221225472u);
        tmp2.lshr(result, zero_one); //Move TT00...0 to right by zero_one -> TTT0...0
        CANAL_ASSERT(tmp2.unsignedMin(res) && res == 0 &&
                     tmp2.unsignedMax(res) && res == 3758096384u);
        tmp2.lshr(one_shift, one_three); //Move 1000...0 to right by one_three -> 0TTT000...0
        CANAL_ASSERT(tmp2.unsignedMin(res) && res == 0 &&
                     tmp2.unsignedMax(res) && res == 1879048192u);
    }

    { //Shift right of 1010101010101...1010
        Integer::Bitfield val(*gEnvironment, llvm::APInt(32, 2863311530u)), tmp(zero);
        CANAL_ASSERT(tmp.lshr(val, zero_one).isTop()); //Move to right by zero to one -> top
        tmp.lshr(val, two); //Move to right by two -> 001010101...1010
        CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == 715827882u);
    }
}

static void
testAshr () {
    Integer::Bitfield one(*gEnvironment, llvm::APInt(32, 1)),
            one_shift(*gEnvironment, llvm::APInt::getSignedMinValue(32)), //10000...000
            bitsize(*gEnvironment, llvm::APInt(32, 32)),
            zero(*gEnvironment, llvm::APInt(32, 0)),
            two(*gEnvironment, llvm::APInt(32, 2)),
            zero_one(zero),
            three(*gEnvironment, llvm::APInt(32, 3)),
            one_three(one),
            top(zero), bottom(zero);
    zero_one.join(one);
    one_three.join(three);
    llvm::APInt res;
    top.setTop();
    bottom.setBottom();

    {
        Integer::Bitfield result(zero);
        CANAL_ASSERT(result.shl(bottom, bottom).isBottom());
        CANAL_ASSERT(result.shl(bottom, top).isBottom());
        CANAL_ASSERT(result.shl(top, bottom).isBottom());
        CANAL_ASSERT(result.shl(bottom, zero).isBottom());
        CANAL_ASSERT(result.shl(zero, bottom).isBottom());

        CANAL_ASSERT(result.shl(top, top).isTop());
        CANAL_ASSERT(result.shl(zero, top).isTop());
        CANAL_ASSERT(result.shl(top, zero).isTop());
    }

    { //Shift rigth for more than bitsize -> 0
        Integer::Bitfield tmp(one);
        tmp.ashr(one_shift, bitsize);
        CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == 0);
    }

    { //Identity
        Integer::Bitfield tmp(zero);
        CANAL_ASSERT(tmp.ashr(three, zero) == three);
    }

    { //Shift one to the right, one bit per iteration
        Integer::Bitfield tmp(one_shift);
        unsigned sum = pow(2, 31);
        for (unsigned i = 1; i < 31; i ++) { //Basic right shift -> propagate one
            Integer::Bitfield t(tmp);
            sum += pow(2, 31 - i);
            tmp.ashr(t, one);
            CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == sum);
        }
    }

    { //Shift one to the right by zero to one -> merge of
        //1000...0
        //1100...0 ->
        //1T00...0
        Integer::Bitfield result(zero), tmp2(zero);
        result.ashr(one_shift, zero_one);
        CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt::getSignedMinValue(32) &&
                     result.unsignedMax(res) && res == 3221225472u);
        tmp2.ashr(result, zero_one); //Move 1T00...0 to right by zero_one -> 1TT0...0
        CANAL_ASSERT(tmp2.unsignedMin(res) && res == llvm::APInt::getSignedMinValue(32) &&
                     tmp2.unsignedMax(res) && res == 3758096384u);
        tmp2.ashr(one_shift, one_three); //Move 1000...0 to right by one_three -> 11TT000...0
        CANAL_ASSERT(tmp2.unsignedMin(res) && res == 3221225472u &&
                     tmp2.unsignedMax(res) && res == 4026531840u);
    }

    { //Shift right of 1010101010101...1010
        Integer::Bitfield val(*gEnvironment, llvm::APInt(32, 2863311530u)), tmp(zero);
        tmp.ashr(val, zero_one); //Move to right by zero to one -> 1TTTTTT
        CANAL_ASSERT(tmp.unsignedMin(res) && res == llvm::APInt::getSignedMinValue(32) &&
                     tmp.unsignedMax(res) && res == llvm::APInt::getMaxValue(32));
        tmp.ashr(val, two); //Move to right by two -> 111010101...1010
        CANAL_ASSERT(tmp.isConstant() && tmp.unsignedMin(res) && res == 3937053354u);
    }
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
            fifteen_sixteen(*gEnvironment, llvm::APInt(32, 15)),
            sixteen(*gEnvironment, llvm::APInt(32, 16)),
            zero_thousand(zero),
            bottom(zero),
            top(zero),
            signedTop(zero),
            unsignedTop(zero),
            signedBottom(zero),
            unsignedBottom(zero),
            signedOverflow(*gEnvironment, llvm::APInt(32, 2147483647)),
            signedOverflow2(*gEnvironment, llvm::APInt::getSignedMinValue(32));
    Integer::Bitfield result(*gEnvironment, 32);
    llvm::APInt res;
    one_two.join(two);
    zero_one.join(one);
    zero_two.join(two);
    minusone_zero.join(zero);
    minustwo_two.join(two);
    zero_thousand.join(thousand);
    fifteen_sixteen.join(sixteen);
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

    result.fromInterval(one_two); //0000TT
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 3));
    result.fromInterval(zero_one);
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 1));
    result.fromInterval(zero_two); //0000TT
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 3));
    result.fromInterval(fifteen_sixteen); //00TTTT
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 31));


    result.fromInterval(minusone_zero); //TOP
    CANAL_ASSERT(result.isTop());
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, -1, true));

    CANAL_ASSERT(result.fromInterval(minustwo_two).isTop()); //-2, -1, 0, 1, 2 -> TOP

    result.fromInterval(zero_thousand); //000...0TTTTTTTTTT
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 1023));

    CANAL_ASSERT(result.fromInterval(signedOverflow).isTop()); //Difference in first bit -> TOP
}

static void
testFPConversions ()
{
    Integer::Bitfield result(*gEnvironment, llvm::APInt(32, 0)),
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

    result.fptoui(five_thousandf);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.signedMax(res) && res == llvm::APInt(32, 1023));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 1023));

    result.fptoui(five_thirtyf);
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.signedMax(res) && res == llvm::APInt(32, 31));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 31));

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

    result.fptosi(five_thousandf); //1111...1111TTTTTTTTTT
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.signedMax(res) && res == llvm::APInt(32, 1023));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 1023));

    result.fptosi(five_thirtyf); //00000....0000TTTTT
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, 0) &&
                 result.signedMax(res) && res == llvm::APInt(32, 31));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, 0) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, 31));

    result.fptosi(minusthousand_minusfivef); //1111...1111TTTTTTTTTT
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -1024, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, -1, true));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, -1024, true) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, -1, true));

    result.fptosi(minusten_minusfivef); //11111....1111111TTTT
    CANAL_ASSERT(result.signedMin(res) && res == llvm::APInt(32, -16, true) &&
                 result.signedMax(res) && res == llvm::APInt(32, -1, true));
    CANAL_ASSERT(result.unsignedMin(res) && res == llvm::APInt(32, -16, true) &&
                 result.unsignedMax(res) && res == llvm::APInt(32, -1, true));

    CANAL_ASSERT(result.fptosi(minusthousand_thousandf).isTop());
    CANAL_ASSERT(result.fptosi(minusten_tenf).isTop());

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
    testShl();
    testLshr();
    testAshr();

    testIntervalConversion();

    testFPConversions();

    delete gEnvironment;
    return 0;
}
