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
    bitfield4.join(bitfield2); //0-1
    bitfield5.join(bitfield1); //0-1
    bitfield6.join(bitfield1); //-1-0
    bitfield9.join(bitfield8); //2-3
    bitfield10.join(bitfield7); //0-2
    bitfield11.join(bitfield8); //1-3

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
    gEnvironment = new Environment(module);

    testJoin();
    testIcmp();

    delete gEnvironment;
    return 0;
}
