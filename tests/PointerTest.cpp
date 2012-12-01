#include "lib/Pointer.h"
#include "lib/Utils.h"
#include "lib/Environment.h"

using namespace Canal;

static Environment *gEnvironment;

static void
testConstructors()
{
    const llvm::PointerType &type =
        *llvm::Type::getInt1PtrTy(gEnvironment->getContext());

    Pointer::Pointer pointer(*gEnvironment, type);
    CANAL_ASSERT(pointer.mTargets.size() == 0);
    CANAL_ASSERT(pointer.isBottom());
}

static void
testEquality()
{
    const llvm::PointerType &type =
        *llvm::Type::getInt1PtrTy(gEnvironment->getContext());

    Pointer::Pointer a(*gEnvironment, type);
    Pointer::Pointer b(*gEnvironment, type);

    // Test empty abstract pointers.
    assert(a.mTargets.size() == 0);
    assert(a == b);
    a.join(b);
    assert(a.mTargets.size() == 0);
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

    delete gEnvironment;
    return 0;
}
