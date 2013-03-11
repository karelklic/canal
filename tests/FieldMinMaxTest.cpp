#include "lib/FieldMinMax.h"
#include "lib/Utils.h"
#include "lib/Environment.h"

#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

#include <stdio.h>

static Canal::Environment *gEnvironment;

using namespace Canal::Field;

static void
testMeet()
{
    Canal::Integer::Interval interval(*gEnvironment, 32);
    interval.setTop();

    MinMax top(interval);
    MinMax bottom(interval);
    bottom.mInterval->setSignedBottom();
    bottom.mInterval->setUnsignedBottom();

    // meet TOP to TOP
    top.meet(top);
    CANAL_ASSERT(top.mInterval->isTop());

    // meet TOP to BOTTOM
    bottom.meet(top);
    CANAL_ASSERT(bottom.mInterval->isSignedBottom());
    CANAL_ASSERT(bottom.mInterval->isUnsignedBottom());

    // meet TOP to <-42,-13> & <13,42>
    MinMax field1(interval);
    field1.mInterval->resetSignedFlags();
    field1.mInterval->mSignedFrom = llvm::APInt(32, -42, true);
    field1.mInterval->mSignedTo = llvm::APInt(32, -13, true);
    field1.mInterval->resetUnsignedFlags();
    field1.mInterval->mUnsignedFrom = llvm::APInt(32, 13);
    field1.mInterval->mUnsignedTo = llvm::APInt(32, 42);

    field1.meet(top);
    CANAL_ASSERT(!field1.mInterval->isTop());
    CANAL_ASSERT(!field1.mInterval->isBottom());
    CANAL_ASSERT(field1.mInterval->mSignedFrom == llvm::APInt(32, -42, true));
    CANAL_ASSERT(field1.mInterval->mSignedTo == llvm::APInt(32, -13, true));
    CANAL_ASSERT(field1.mInterval->mUnsignedFrom == llvm::APInt(32, 13));
    CANAL_ASSERT(field1.mInterval->mUnsignedTo == llvm::APInt(32, 42));

    // meet <-42,-13> & <13,42> to TOP
    top.meet(field1);
    CANAL_ASSERT(!top.mInterval->isTop());
    CANAL_ASSERT(!top.mInterval->isBottom());
    CANAL_ASSERT(top.mInterval->mSignedFrom == llvm::APInt(32, -42, true));
    CANAL_ASSERT(top.mInterval->mSignedTo == llvm::APInt(32, -13, true));
    CANAL_ASSERT(top.mInterval->mUnsignedFrom == llvm::APInt(32, 13));
    CANAL_ASSERT(top.mInterval->mUnsignedTo == llvm::APInt(32, 42));

    // meet <-42,13> & <13,42> to <-20,-5> & <5,20>
    MinMax field2(interval);
    field2.mInterval->resetSignedFlags();
    field2.mInterval->mSignedFrom = llvm::APInt(32, -20, true);
    field2.mInterval->mSignedTo = llvm::APInt(32, -5, true);
    field2.mInterval->resetUnsignedFlags();
    field2.mInterval->mUnsignedFrom = llvm::APInt(32, 5);
    field2.mInterval->mUnsignedTo = llvm::APInt(32, 20);

    field2.meet(field1);
    CANAL_ASSERT(!field2.mInterval->isBottom());
    CANAL_ASSERT(!field2.mInterval->isTop());
    CANAL_ASSERT(field2.mInterval->mSignedFrom == llvm::APInt(32, -20, true));
    CANAL_ASSERT(field2.mInterval->mSignedTo == llvm::APInt(32, -13, true));
    CANAL_ASSERT(field2.mInterval->mUnsignedFrom == llvm::APInt(32, 13));
    CANAL_ASSERT(field2.mInterval->mUnsignedTo == llvm::APInt(32, 20));
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Canal::Environment(module);

    testMeet();

    delete gEnvironment;
    return 0;
}
