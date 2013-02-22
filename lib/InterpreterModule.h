#ifndef LIBCANAL_INTERPRETER_MODULE_H
#define LIBCANAL_INTERPRETER_MODULE_H

#include "State.h"
#include <vector>
#include <string>

namespace Canal {

class Constructors;
class Environment;

namespace Interpreter {

class Function;

class Module
{
    const llvm::Module &mModule;

    const Environment &mEnvironment;

    // State of all functions is kept here.
    // Workers iterate on functions until the fixpoint is reached.
    std::vector<Function*> mFunctions;

    State mGlobalState;

public:
    Module(const llvm::Module &module,
           const Constructors &constructors);

    virtual ~Module();

    std::vector<Function*>::const_iterator begin() const
    {
        return mFunctions.begin();
    }

    std::vector<Function*>::const_iterator end() const
    {
        return mFunctions.end();
    }

    bool empty() const
    {
        return mFunctions.empty();
    }

    Function *getFunction(const char *name) const;

    Function *getFunction(const std::string &name) const
    {
        return getFunction(name.c_str());
    }

    Function *getFunction(const llvm::Function &function) const;

    std::string toString() const;

    void dumpToMetadata() const;

    void updateGlobalState();
};

} // namespace Interpreter
} // namespace Canal

#endif // LIBCANAL_INTERPRETER_FUNCTION_H
