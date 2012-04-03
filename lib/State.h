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

class AbstractValue;

// llvm::Value represents an output from an instruction
// (llvm::Instruction), or some global value (llvm::Constant).
typedef std::map<const llvm::Value*, AbstractValue*> VariablesMap;
typedef std::vector<AbstractValue*> MemoryBlockList;

// Includes global variables and heap.
// Includes function-level memory and variables (=stack).
class State
{
public:
    // The key (llvm::Value*) is not owned by this class.  It is not
    // deleted.  The value (AbstractValue*) memory is owned by this
    // class, so it is deleted in state destructor.
    VariablesMap mGlobalVariables;

    // Nameless memory/values allocated on the heap.  It's referenced
    // either by a pointer somewhere on a stack, by a global variable,
    // or by another Block or stack Block.
    //
    // The members of the list are owned by this class, so they are
    // deleted in the state destructor.
    MemoryBlockList mGlobalBlocks;

    // The value pointer does _not_ point to StackBlocks! To connect
    // with a StackBlocks item, create an AbstractPointer value object
    // which contains a pointer to a StackBlocks item.
    //
    // The key (llvm::Value*) is not owned by this class.  It is not
    // deleted.  The value (AbstractValue*) memory is owned by this
    // class, so it is deleted in state destructor.
    VariablesMap mFunctionVariables;

    // Nameless memory/values allocated on the stack.  It's referenced
    // either by a pointer in StackVariables or GlobalVariables, or by
    // another Block or heap Block.
    //
    // The members of the list are owned by this class, so they are
    // deleted in the state destructor.
    MemoryBlockList mFunctionBlocks;

    State();

    State(const State &rhs);

    virtual ~State();

    State &operator=(const State &rhs);

    bool operator==(const State &rhs) const;
    bool operator!=(const State &rhs) const;

    void clear();

    void merge(const State &state);
};

// Support writing of operational state to output stream.  Used for
// logging purposes.
llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
			      const State &state);

} // namespace Canal

#endif // CANAL_STATE_H
