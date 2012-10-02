#ifndef LIBCANAL_INTERPRETER_BLOCK_MODULE_H
#define LIBCANAL_INTERPRETER_BLOCK_MODULE_H

#include <vector>
#include <string>

namespace llvm {
class Module;
class Function;
} // namespace llvm

namespace Canal {

class Constructors;

namespace InterpreterBlock {

class Function;

class Module
{
public:
    Module(const llvm::Module &module,
           const Constructors &constructors);

    virtual ~Module();

    std::vector<Function*>::const_iterator begin() const { return mFunctions.begin(); }

    std::vector<Function*>::const_iterator end() const { return mFunctions.end(); }

    Function *getFunction(const char *name) const;

    Function *getFunction(const std::string &name) const { return getFunction(name.c_str()); }

    Function *getFunction(const llvm::Function &function) const;

    std::string toString() const;

protected:
    const llvm::Module &mModule;

    // State of all functions is kept here.
    // Workers iterate on functions until the fixpoint is reached.
    std::vector<Function*> mFunctions;
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H
