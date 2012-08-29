#include "lib/Pointer.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

using namespace Canal;

static void
testConstructors(const Environment &environment)
{
    const llvm::PointerType &type =
        *llvm::Type::getInt1PtrTy(environment.getContext());

    Pointer::InclusionBased pointer(environment, type);
    CANAL_ASSERT(pointer.mTargets.size() == 0);
    CANAL_ASSERT(!pointer.isTop());
    CANAL_ASSERT(pointer.isBottom());
}

static void
testEquality(const Environment &environment)
{
    const llvm::PointerType &type =
        *llvm::Type::getInt1PtrTy(environment.getContext());

    Pointer::InclusionBased a(environment, type), b(environment, type);

    // Test empty abstract pointers.
    assert(a.mTargets.size() == 0);
    assert(a == b);
    a.merge(b);
    assert(a.mTargets.size() == 0);
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module module("testModule", context);
    Environment environment(module);

    testConstructors(environment);
    testEquality(environment);

    return 0;
}
