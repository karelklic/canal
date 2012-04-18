#ifndef CANAL_STATE_H
#define CANAL_STATE_H

#include <map>
#include <vector>
#include <string>

namespace llvm {
    class Value;
    class raw_ostream;
}

namespace Canal {

class Value;

// llvm::Value represents a place in the program (an instruction,
// instance of llvm::Instruction).
typedef std::map<const llvm::Value*, Value*> PlaceValueMap;

// Includes global variables and heap.
// Includes function-level memory and variables (=stack).
class State
{
public:
    State();
    State(const State &state);
    virtual ~State();

    State &operator=(const State &state);

    bool operator==(const State &state) const;
    bool operator!=(const State &state) const;

    void clear();

    void merge(const State &state);

    void addGlobalVariable(const llvm::Value *instruction, Value *value);
    void addFunctionVariable(const llvm::Value *instruction, Value *value);

    void addGlobalBlock(const llvm::Value *instruction, Value *value);
    void addFunctionBlock(const llvm::Value *instruction, Value *value);

    const PlaceValueMap &getGlobalVariables() const { return mGlobalVariables; }
    const PlaceValueMap &getGlobalBlocks() const { return mGlobalBlocks; }
    const PlaceValueMap &getFunctionVariables() const { return mFunctionVariables; }
    const PlaceValueMap &getFunctionBlocks() const { return mFunctionBlocks; }

    // Search both global and function variables for a place.  If the
    // place is found, the variable is returned.  Otherwise NULL is
    // returned.
    Value *findVariable(const llvm::Value *place) const;

protected:
    // The key (llvm::Value*) is not owned by this class.  It is not
    // deleted.  The value (Value*) memory is owned by this
    // class, so it is deleted in state destructor.
    PlaceValueMap mGlobalVariables;

    // Nameless memory/values allocated on the heap.  It's referenced
    // either by a pointer somewhere on a stack, by a global variable,
    // or by another Block or stack Block.
    //
    // The keys are not owned by this class.  They represent the place
    // where the block has been allocated.  The values are owned by
    // this class, so they are deleted in the state destructor.
    PlaceValueMap mGlobalBlocks;

    // The value pointer does _not_ point to mFunctionBlocks! To connect
    // with a mFunctionBlocks item, create a Pointer object that
    // contains a pointer to a StackBlocks item.
    //
    // The key (llvm::Value*) is not owned by this class.  It is not
    // deleted.  The value (Value*) memory is owned by this
    // class, so it is deleted in state destructor.
    PlaceValueMap mFunctionVariables;

    // Nameless memory/values allocated on the stack.  The values are
    // referenced either by a pointer in mFunctionVariables or
    // mGlobalVariables, or by another item in mFunctionBlocks or
    // mGlobalBlocks.
    //
    // The members of the list are owned by this class, so they are
    // deleted in the state destructor.
    PlaceValueMap mFunctionBlocks;

public:
    // Value returned from function.
    Value *mReturnedValue;
};

// Support writing of operational state to output stream.  Used for
// logging purposes.
llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
			      const State &state);

} // namespace Canal

#endif // CANAL_STATE_H
