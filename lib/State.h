#ifndef LIBCANAL_STATE_H
#define LIBCANAL_STATE_H

#include <map>
#include <vector>
#include <string>

namespace llvm {
class Function;
class Value;
class raw_ostream;
} // namespace llvm

namespace Canal {

class Domain;
class SlotTracker;

/// llvm::Value represents a place in the program (an instruction,
/// instance of llvm::Instruction).
typedef std::map<const llvm::Value*, Domain*> PlaceValueMap;

/// @brief Abstract memory state.
///
/// Consists of function-level variables (also called registers) and
/// stack memory, global variables and heap, and return value.  All
/// variables are in abstract domain.
class State
{
public:
    State();
    State(const State &state);
    virtual ~State();

    State &operator=(const State &state);

    bool operator==(const State &state) const;
    bool operator!=(const State &state) const;

    /// Clears everything.  Releases all memory.
    void clear();

    /// Clears function variables, blocks and returned value.
    void clearFunctionLevel();

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

    const PlaceValueMap &getGlobalVariables() const { return mGlobalVariables; }
    const PlaceValueMap &getGlobalBlocks() const { return mGlobalBlocks; }
    const PlaceValueMap &getFunctionVariables() const { return mFunctionVariables; }
    const PlaceValueMap &getFunctionBlocks() const { return mFunctionBlocks; }

    /// Search both global and function variables for a place.  If the
    /// place is found, the variable is returned.  Otherwise NULL is
    /// returned.
    Domain *findVariable(const llvm::Value &place) const;

    /// Search both global and function blocks for a place.  If the
    /// place is found, the block is returned.  Otherwise NULL is
    /// returned.
    Domain *findBlock(const llvm::Value &place) const;

    std::string toString(const llvm::Value &place,
                         SlotTracker &slotTracker) const;

protected:
    /// The key (llvm::Value*) is not owned by this class.  It is not
    /// deleted.  The value (Domain*) memory is owned by this
    /// class, so it is deleted in state destructor.
    PlaceValueMap mGlobalVariables;

    /// Nameless memory/values allocated on the heap.  It's referenced
    /// either by a pointer somewhere on a stack, by a global variable,
    /// or by another Block or stack Block.
    ///
    /// The keys are not owned by this class.  They represent the place
    /// where the block has been allocated.  The values are owned by
    /// this class, so they are deleted in the state destructor.
    PlaceValueMap mGlobalBlocks;

    /// The value pointer does _not_ point to mFunctionBlocks! To connect
    /// with a mFunctionBlocks item, create a Pointer object that
    /// contains a pointer to a StackBlocks item.
    ///
    /// The key (llvm::Value*) is not owned by this class.  It is not
    /// deleted.  The value (Domain*) memory is owned by this
    /// class, so it is deleted in state destructor.
    PlaceValueMap mFunctionVariables;

    /// Nameless memory/values allocated on the stack.  The values are
    /// referenced either by a pointer in mFunctionVariables or
    /// mGlobalVariables, or by another item in mFunctionBlocks or
    /// mGlobalBlocks.
    ///
    /// The members of the list are owned by this class, so they are
    /// deleted in the state destructor.
    PlaceValueMap mFunctionBlocks;

public:
    /// Value returned from function.
    Domain *mReturnedValue;
};

} // namespace Canal

#endif // LIBCANAL_STATE_H
