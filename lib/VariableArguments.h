#ifndef LIBCANAL_VARIABLE_ARGUMENTS_H
#define LIBCANAL_VARIABLE_ARGUMENTS_H

#include "Prereq.h"
#include <map>
#include <vector>

namespace Canal {

class Domain;

class VariableArguments
{
    // llvm::Instruction represents the calling instruction providing
    // the variable arguments.
    typedef std::map<const llvm::Instruction*, std::vector<Domain*> > CallMap;

    CallMap mCalls;

public:
    /// Standard constructor.
    VariableArguments() {}

    /// Copy constructor.  Makes a deep copy of all arguments.
    VariableArguments(const VariableArguments &arguments);

    /// Standard destructor.  Deletes all arguments.
    ~VariableArguments();

    bool operator==(const VariableArguments &arguments) const;

    /// Merges the arguments per every instruction.
    void merge(const VariableArguments &arguments);

    /// Adds an argument at the end of the argument list for an
    /// instruction.
    void addArgument(const llvm::Instruction &place, Domain *argument);

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments.  Do not implement!
    VariableArguments &operator=(const VariableArguments &arguments);
};

} // namespace Canal

#endif // LIBCANAL_VARIABLE_ARGUMENTS_H
