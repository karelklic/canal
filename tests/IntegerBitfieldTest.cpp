#include "lib/IntegerBitfield.h"
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
        bitfield11(bitfield2);

    llvm::APInt res;

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
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testJoin();
    testIcmp();

    delete gEnvironment;
    return 0;
}
