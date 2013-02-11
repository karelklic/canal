#include "lib/ArrayStringTrie.h"
#include "lib/Environment.h"
#include "lib/Utils.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

using namespace Canal;

static Environment *gEnvironment;

static void testConstructors()
{
    const llvm::ArrayType &type = *llvm::ArrayType::get(llvm::Type::getInt8Ty(
        gEnvironment->getContext()), 10);

    Array::StringTrie stringTrie(*gEnvironment, type);
    CANAL_ASSERT(stringTrie.isBottom());
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;

    llvm::Module *module = new llvm::Module("testModule", context);
    gEnvironment = new Environment(module);

    testConstructors();

    delete gEnvironment;
    return 0;
}

