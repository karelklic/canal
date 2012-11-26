#include "lib/ArrayExactSize.h"
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
    Integer::Interval interval(*gEnvironment, 32);
    Array::ExactSize array(*gEnvironment, 10, interval);
    CANAL_ASSERT(array.isBottom());
    CANAL_ASSERT(array.size() == 10);
    CANAL_ASSERT(array == array);
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();

    delete gEnvironment;
    return 0;
}
