#ifndef LIBCANAL_STATE_H
#define LIBCANAL_STATE_H

#include "VariableArguments.h"
#include "StateMap.h"
#include <string>

namespace Canal {

class Domain;
class SlotTracker;

/// @brief Abstract memory state.
///
/// Consists of function-level variables (also called registers) and
/// stack memory, global variables and heap, and return value.  All
/// variables are in abstract domain.
class State
{
    /// The key (llvm::Value*) is not owned by this class.  It is not
    /// deleted.  The value (Domain*) memory is owned by this
    /// class, so it is deleted in state destructor.
    StateMap mGlobalVariables;

    /// Nameless memory/values allocated on the heap.  It's referenced
    /// either by a pointer somewhere on a stack, by a global variable,
    /// or by another Block or stack Block.
    ///
    /// The keys are not owned by this class.  They represent the place
    /// where the block has been allocated.  The values are owned by
    /// this class, so they are deleted in the state destructor.
    StateMap mGlobalBlocks;

    /// The value pointer does _not_ point to mFunctionBlocks! To connect
    /// with a mFunctionBlocks item, create a Pointer object that
    /// contains a pointer to a StackBlocks item.
    ///
    /// The key (llvm::Value*) is not owned by this class.  It is not
    /// deleted.  The value (Domain*) memory is owned by this
    /// class, so it is deleted in state destructor.
    StateMap mFunctionVariables;

    /// Nameless memory/values allocated on the stack.  The values are
    /// referenced either by a pointer in mFunctionVariables or
    /// mGlobalVariables, or by another item in mFunctionBlocks or
    /// mGlobalBlocks.
    ///
    /// The members of the list are owned by this class, so they are
    /// deleted in the state destructor.
    StateMap mFunctionBlocks;

    /// Value returned from function.
    Domain *mReturnedValue;

    /// Variable arguments added to the function.
    VariableArguments mVariableArguments;

public:
    State();

    State(const State &state);

    virtual ~State();

    bool operator==(const State &state) const;
    bool operator!=(const State &state) const;

    /// Merge everything.
    void merge(const State &state);

    /// Merge global variables and blocks.
    void mergeGlobal(const State &state);

    /// Merge the returned value.
    void mergeReturnedValue(const State &state);

    /// Merge function blocks only.
    void mergeFunctionBlocks(const State &state);

    /// Merge function memory blocks external to a function.
    /// This is used after a function call, where the modifications of
    /// the global state need to be merged to the state of the caller,
    /// but its local state is not relevant.
    void mergeForeignFunctionBlocks(const State &state,
                                    const llvm::Function &currentFunction);

    /// @param place
    ///   Represents a place in the program where the global variable
    ///   is defined and assigned.
    void addGlobalVariable(const llvm::Value &place, Domain *value);

    /// Adds a register-type value to the stack.
    /// @param place
    ///   Represents a place in the program where the function variable
    ///   is assigned.  Usually it is an instance of llvm::Instruction
    ///   for a result of the instruction.  It might also be an
    ///   instance of llvm::Argument, which represents a function call
    ///   parameter.
    /// @see
    ///   To add a value created by alloca to the stack, use the method
    ///   addFunctionBlock.
    void addFunctionVariable(const llvm::Value &place, Domain *value);

    void addGlobalBlock(const llvm::Value &place, Domain *value);

    /// Adds a value created by alloca to the stack.
    void addFunctionBlock(const llvm::Value &place, Domain *value);

    void setReturnedValue(Domain *value);

    void mergeToReturnedValue(const Domain &value);

    const Domain *getReturnedValue() const
    {
        return mReturnedValue;
    }

    void addVariableArgument(const llvm::Instruction &place, Domain *argument);

    const StateMap &getGlobalVariables() const
    {
        return mGlobalVariables;
    }

    StateMap &getGlobalVariables()
    {
        return mGlobalVariables;
    }

    const StateMap &getGlobalBlocks() const
    {
        return mGlobalBlocks;
    }

    StateMap &getGlobalBlocks()
    {
        return mGlobalBlocks;
    }

    const StateMap &getFunctionVariables() const
    {
        return mFunctionVariables;
    }

    StateMap &getFunctionVariables()
    {
        return mFunctionVariables;
    }

    const StateMap &getFunctionBlocks() const
    {
        return mFunctionBlocks;
    }

    StateMap &getFunctionBlocks()
    {
        return mFunctionBlocks;
    }

    /// Search both global and function variables for a place.  If the
    /// place is found, the variable is returned.  Otherwise NULL is
    /// returned.
    const Domain *findVariable(const llvm::Value &place) const;

    /// Search both global and function blocks for a place.  If the
    /// place is found, the block is returned.  Otherwise NULL is
    /// returned.
    const Domain *findBlock(const llvm::Value &place) const;

    bool hasGlobalBlock(const llvm::Value &place) const;

    /// Get memory usage (used byte count) of this abstract state.
    size_t memoryUsage() const;

    std::string toString(const llvm::Value &place,
                         SlotTracker &slotTracker) const;

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of states.  Do not implement!
    State &operator=(const State &state);
};

} // namespace Canal

#endif // LIBCANAL_STATE_H
