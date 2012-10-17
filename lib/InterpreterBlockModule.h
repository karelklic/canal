#ifndef LIBCANAL_INTERPRETER_BLOCK_MODULE_H
#define LIBCANAL_INTERPRETER_BLOCK_MODULE_H

#include "State.h"
#include <vector>
#include <string>
#include <llvm/GlobalVariable.h>

namespace llvm {
class Module;
class Function;
} // namespace llvm

namespace Canal {

class Constructors;
class Environment;

namespace InterpreterBlock {

class Function;

class Module
{
protected:
    const llvm::Module &mModule;
    const Environment &mEnvironment;

    // State of all functions is kept here.
    // Workers iterate on functions until the fixpoint is reached.
    std::vector<Function*> mFunctions;

    State mGlobalState;

private:
    //Topological sort of global variables
    typedef struct {
        const llvm::Value* constant;
        unsigned count;
    } tsortValue;

    llvm::GlobalVariable* nextGlobalVariable();
    void tsortInit();
    bool tsortDepend(const llvm::GlobalVariable& what, tsortValue *value);
    bool tsortDepend(const llvm::Constant& what, tsortValue* value);
    void tsortDecrement(tsortValue*& value);
    const llvm::GlobalVariable* tsortNext();

    std::vector<const llvm::GlobalVariable*> mTsortReady;
    std::map<const llvm::Value*, std::vector<tsortValue*> > mTsortDependencies;


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

    void updateGlobalState();
};

} // namespace InterpreterBlock
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_BLOCK_FUNCTION_H
