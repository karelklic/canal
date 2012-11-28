#include "lib/IntegerEnumeration.h"
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
    Integer::Enumeration enumeration1(*gEnvironment, llvm::APInt(32, 0)),
        enumeration2(*gEnvironment, llvm::APInt(32, 1)),
        enumeration3(*gEnvironment, negone),
        enumeration4(enumeration1),
        enumeration5(enumeration2),
        enumeration6(enumeration3),
        result(enumeration1),
        enumeration7(*gEnvironment, llvm::APInt(32, 2)),
        enumeration8(*gEnvironment, llvm::APInt(32, 3)),
        enumeration9(enumeration7),
        enumeration10(enumeration1),
        enumeration11(enumeration1);

    llvm::APInt res;

    enumeration4.join(enumeration2); //0-1
    CANAL_ASSERT(enumeration4.signedMin(res) && res == 0 && enumeration4.signedMax(res) && res == 1);

    enumeration4.join(enumeration4); //Join with itself
    CANAL_ASSERT(enumeration4.signedMin(res) && res == 0 && enumeration4.signedMax(res) && res == 1);

    enumeration5.join(enumeration1); //0-1
    CANAL_ASSERT(enumeration5.signedMin(res) && res == 0 && enumeration5.signedMax(res) && res == 1);

    enumeration6.join(enumeration1); //-1-0
    CANAL_ASSERT(enumeration6.signedMin(res) && res == negone && enumeration6.signedMax(res) && res == 0);
    CANAL_ASSERT(enumeration6.unsignedMin(res) && res == 0 && enumeration6.unsignedMax(res) && res == negone);

    enumeration5.join(enumeration6); //-1-1
    CANAL_ASSERT(enumeration5.signedMin(res) && res == negone && enumeration5.signedMax(res) && res == 1);
    CANAL_ASSERT(enumeration5.unsignedMin(res) && res == 0 && enumeration5.unsignedMax(res) && res == negone); //0-(-1)

    enumeration9.join(enumeration8); //2-3
    CANAL_ASSERT(enumeration9.signedMin(res) && res == 2 && enumeration9.signedMax(res) && res == 3);

    enumeration6.join(enumeration8); //-1-3
    CANAL_ASSERT(enumeration6.signedMin(res) && res == negone && enumeration6.signedMax(res) && res == 3);
    CANAL_ASSERT(enumeration6.unsignedMin(res) && res == 0 && enumeration6.unsignedMax(res) && res == negone); //0-(-1)

    enumeration10.setTop(); //Top
    CANAL_ASSERT(enumeration10.isTop());

    enumeration10.join(enumeration10); //Join with itself
    CANAL_ASSERT(enumeration10.isTop());

    enumeration10.join(enumeration4); //Join with any enumeration
    CANAL_ASSERT(enumeration10.isTop());

    //Test of overflow
    for (unsigned i = 0; i < 100000; i ++) {
        enumeration11.join(Integer::Enumeration(*gEnvironment, llvm::APInt(32, i)));
        if (enumeration11.isTop()) break;
    }
    CANAL_ASSERT(enumeration11.isTop());
}

static void
testIcmp()
{
    Integer::Enumeration enumeration1(*gEnvironment, llvm::APInt(32, 0)),
        enumeration2(*gEnvironment, llvm::APInt(32, 1)),
        enumeration3(*gEnvironment, llvm::APInt(32, -1, true)),
        enumeration4(enumeration1),
        enumeration5(enumeration2),
        enumeration6(enumeration3),
        result(enumeration1),
        enumeration7(*gEnvironment, llvm::APInt(32, 2)),
        enumeration8(*gEnvironment, llvm::APInt(32, 3)),
        enumeration9(enumeration7),
        enumeration10(enumeration1),
        enumeration11(enumeration2);

    llvm::APInt res;
    enumeration4.join(enumeration2); //0-1
    enumeration5.join(enumeration1); //0-1
    enumeration6.join(enumeration1); //-1-0
    enumeration9.join(enumeration8); //2-3
    enumeration10.join(enumeration7); //0-2
    enumeration11.join(enumeration8); //1-3

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_EQ); //0 != 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_NE); //0 != 1
    CANAL_ASSERT(result.isTrue());

    //Difference from interval
    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_EQ); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> false
    CANAL_ASSERT(result.isFalse()); //False


    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_NE); //-1-0 ? 1 (when unsigned, -1-0 is 0-max) -> true
    CANAL_ASSERT(result.isTrue()); //True

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_EQ); //0-1 != 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_NE); //0-1 != 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_EQ); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration5, llvm::CmpInst::ICMP_NE); //0-1 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_EQ); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_NE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_EQ); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_NE); //0-1 ==  0-1 (same object)
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_EQ); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_NE); //0-1 ? 0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration4, llvm::CmpInst::ICMP_EQ); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration4, llvm::CmpInst::ICMP_NE); //0 ? 0-1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_EQ); //0-1 != 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_NE); //0-1 != 2-3
    CANAL_ASSERT(result.isTrue());

    //Difference from interval
    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_EQ); //0-2 != 1-3 (no intersection)
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration10, enumeration11, llvm::CmpInst::ICMP_NE); //0-2 != 1-3 (no intersection)
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_SGT); //0 > 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_SGE); //0 > 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_UGT); //0 > 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_UGE); //0 > 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_SLT); //0 > 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_SLE); //0 > 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_ULT); //0 > 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration1, enumeration2, llvm::CmpInst::ICMP_ULE); //0 > 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_SLE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_SLT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_ULE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_ULT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_SGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_SGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_UGE); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration2, enumeration6, llvm::CmpInst::ICMP_UGT); //1 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >, unsigned top
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_SLE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_SLT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_ULE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_ULT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_SGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_SGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_UGE); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration1, enumeration6, llvm::CmpInst::ICMP_UGT); //0 ? -1-0 (when unsigned, -1-0 is 0-max) -> signed >=, unsigned <=
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_SGE); //0-1 < 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_SGT); //0-1 < 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_UGE); //0-1 < 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_UGT); //0-1 < 2
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_SLE); //0-1 < 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_SLT); //0-1 < 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_ULE); //0-1 < 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration7, llvm::CmpInst::ICMP_ULT); //0-1 < 2
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_SGE); //0-1 <= 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_SGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_UGE); //0-1 <= 1
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_UGT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_SLE); //0-1 <= 1
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_SLT); //0-1 <= 1
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration1, llvm::CmpInst::ICMP_ULE); //0-1 <= 1
    CANAL_ASSERT(result.isFalse());

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
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_ULT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_ULE); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_SLT); //0-1 ? -1-0
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration6, llvm::CmpInst::ICMP_SLE); //0-1 <= -1-0 //Signed
    CANAL_ASSERT(result.isFalse());


    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_SGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_SGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_UGT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_UGE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_SLT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_SLE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_ULT); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTop());

    result.icmp(enumeration4, enumeration4, llvm::CmpInst::ICMP_ULE); //0-1 == 0-1 (same object)
    CANAL_ASSERT(result.isTrue());


    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_SGE); //0-1 < 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_SGT); //0-1 < 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_UGE); //0-1 < 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_UGT); //0-1 < 2-3
    CANAL_ASSERT(result.isFalse());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_SLE); //0-1 < 2-3
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_SLT); //0-1 < 2-3
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_ULE); //0-1 < 2-3
    CANAL_ASSERT(result.isTrue());

    result.icmp(enumeration4, enumeration9, llvm::CmpInst::ICMP_ULT); //0-1 < 2-3
    CANAL_ASSERT(result.isTrue());

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
