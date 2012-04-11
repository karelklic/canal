#ifndef CANAL_VALUE_H
#define CANAL_VALUE_H

#include <cstddef>

namespace llvm {
    class raw_ostream;
}

namespace Canal {

class State;

class Value
{
public:
    // Create a copy of this value.
    virtual Value *clone() const = 0;

    // Implementing this is mandatory.  Values are compared while
    // computing the fixed point.
    virtual bool operator==(const Value &value) const = 0;
    // Inequality is implemented by calling the equality operator.
    virtual bool operator!=(const Value &value) const;

    // Merge another value into this one.
    virtual void merge(const Value &value);

    // Get memory usage (used byte count) of this value.
    virtual size_t memoryUsage() const = 0;

    // Write to output stream for logging purposes.
    virtual void printToStream(llvm::raw_ostream &ostream) const = 0;

public:
    // Implementation of instructions operating on values.

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
};

// Support writing of abstract values to output stream.  Used for
// logging purposes.
llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
                              const Value &value);


// Base class for abstract states that require to know not just its
// own value, but the complete state where they belong to.  This is
// currently used for pointers, as pointers point to other objects.
class StateValue
{
public:
    StateValue() : mState(NULL) {}

    State *getState() const { return mState; }
    void setState(State *state) { mState = state; }

protected:
    State *mState;
};

// Base class for abstract states that can inform about accuracy.
class AccuracyValue
{
public:
    // Get accuracy of the abstract value (0 - 1). In finite-height
    // lattices, it is determined by the position of the value in the
    // lattice.
    //
    // Accuracy 0 means that the value represents all possible values
    // (top).  Accuracy 1 means that the value represents the most
    // precise and exact value (bottom).
    virtual float accuracy() const;

    // Is it the lowest value.
    virtual bool isBottom() const;
    // Set to the lowest value.
    virtual void setBottom();

    // Is it the highest value.
    virtual bool isTop() const;
    // Set it to the top value of lattice.
    virtual void setTop();
};

// Base class for abstract values that can lower the precision and
// memory requirements on demand.
class VariablePrecisionValue
{
public:
    // Decrease memory usage of this value below the provided size in
    // bytes.  Returns true if the memory usage was limited, false when
    // it was not possible.
    virtual bool limitMemoryUsage(size_t size);
};

} // namespace Canal

#endif // CANAL_VALUE_H
