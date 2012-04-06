#ifndef CANAL_ABSTRACT_VALUE_H
#define CANAL_ABSTRACT_VALUE_H

#include <cstddef>

namespace llvm {
    class raw_ostream;
}

namespace Canal {

class State;

class Value
{
public:
    // Constructor that sets bottom value
    Value() {}

    // Create a copy of this value.
    virtual Value *clone() const = 0;

    virtual bool operator==(const Value &rhs) const = 0;
    virtual bool operator!=(const Value &rhs) const;

    // Merge another value into this one.
    virtual void merge(const Value &v);

    // Get memory usage (used byte count) of this value.
    virtual size_t memoryUsage() const = 0;

    // Decrease memory usage of this value below the provided size in
    // bytes.  Returns true if the memory usage was limited, false when
    // it was not possible.
    virtual bool limitMemoryUsage(size_t size);

    // Get accuracy of the abstract value (0 - 1). In finite-height
    // lattices, it is determined by the position of the value in the
    // lattice.
    //
    // Accuracy 0 means that the value represents all possible values.
    // Accuracy 1 means that the value represents the most precise and
    // exact value.
    virtual float accuracy() const = 0;

    // Is it the lowest value.
    virtual bool isBottom() const;
    // Set it to the top value of lattice.
    virtual void setTop();

    virtual void add(const Value &a, const Value &b);
    virtual void sub(const Value &a, const Value &b);
    virtual void mul(const Value &a, const Value &b);
    // Signed division.
    virtual void div(const Value &a, const Value &b);
    // Signed remainder.
    virtual void rem(const Value &a, const Value &b);
    virtual void shl(const Value &a, const Value &b);
    virtual void shr(const Value &a, const Value &b);
    virtual void and_(const Value &a, const Value &b);
    virtual void or_(const Value &a, const Value &b);
    virtual void xor_(const Value &a, const Value &b);

    // Write to output stream fro logging purposes.
    virtual void printToStream(llvm::raw_ostream &ostream) const = 0;
};

// Support writing of abstract values to output stream.  Used for
// logging purposes.
llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
                              const Value &value);


// Base class for abstract states that require to know not just its
// own value, but the complete state where they belong to.  This is
// currently used for pointers, as pointers point to other objects.
class StateOwner
{
public:
    StateOwner() : mState(NULL) {}

    State *getState() const { return mState; }
    void setState(State *state) { mState = state; }

protected:
    State *mState;
};

} // namespace Canal

#endif // CANAL_ABSTRACT_VALUE_H
