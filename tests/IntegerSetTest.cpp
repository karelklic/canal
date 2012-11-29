#include "lib/IntegerSet.h"
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
        set11(set1);

    llvm::APInt res;

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
