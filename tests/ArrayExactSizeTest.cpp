#include "lib/ArrayExactSize.h"
#include "lib/IntegerInterval.h"
#include "lib/Utils.h"
#include "lib/Environment.h"
#include "lib/Interpreter.h"
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/ManagedStatic.h>

using namespace Canal;

static Interpreter::Interpreter *gInterpreter;

static void
testConstructors()
{
    const llvm::ArrayType &typeI10 = *llvm::ArrayType::get(llvm::Type::getInt32Ty(
        gInterpreter->getEnvironment().getContext()), 10);

    Array::ExactSize array(gInterpreter->getEnvironment(), typeI10);
    CANAL_ASSERT(array.isBottom());
    CANAL_ASSERT(array == array);
}

int
main(int argc, char **argv)
{
    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y; // Call llvm_shutdown() on exit.
    llvm::Module *module = new llvm::Module("testModule", context);
    gInterpreter = new Interpreter::Interpreter(module);

    testConstructors();

    delete gInterpreter;
    return 0;
}
