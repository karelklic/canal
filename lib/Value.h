#ifndef LIBCANAL_VALUE_H
#define LIBCANAL_VALUE_H

#include <cstddef>
#include <string>
#include <llvm/Instructions.h>

namespace llvm {
class raw_ostream;
} // namespace llvm

namespace Canal {

class State;

/// @brief
/// Base class for all abstract domains.
class Value
{
public:
    typedef void(Value::*CastOperation)(const Value&);

    typedef void(Value::*BinaryOperation)(const Value&,
                                          const Value&);

    typedef void(Value::*CmpOperation)(const Value&,
                                       const Value&,
                                       llvm::CmpInst::Predicate predicate);

    /// Prepare value so that merge will not fail on assert when what is Constant.
    static Value* handleMergeConstants(Value *what, const Value* target);
public:
    /// Create a copy of this value.
    virtual Value *clone() const = 0;

    /// This is used to obtain instance of the value type and to get an
    /// empty value at the same time.
    virtual Value *cloneCleaned() const = 0;

    /// Implementing this is mandatory.  Values are compared while
    /// computing the fixed point.
    virtual bool operator==(const Value &value) const = 0;
    /// Inequality is implemented by calling the equality operator.
    virtual bool operator!=(const Value &value) const;

    /// Merge another value into this one.
    virtual void merge(const Value &value);

    /// Get memory usage (used byte count) of this abstract value.
    virtual size_t memoryUsage() const = 0;

    /// An idea for different memory interpretation.
    /// virtual Value *castTo(const llvm::Type *itemType, int offset) const = 0;

    /// Create a string representation of the abstract value.
    virtual std::string toString() const = 0;

    /// Checks if the abstract value internal state matches the text
    /// description.  Full coverage of the state is not expected, the
    /// text can contain just partial information.
    virtual bool matchesString(const std::string &text,
                               std::string &rationale) const = 0;

    /// Load the abstract value state from a string representation.
    /// @param text
    ///   The textual representation.  It must not contain any text
    ///   that does not belong to this abstract value state.
    /// @returns
    ///   True if the text has been successfully parsed and the state
    ///   has been set from the text.  False otherwise.
    //virtual bool fromString(const std::string &text) = 0;

public:
    /// Implementation of instructions operating on values.
    virtual void add(const Value &a, const Value &b);
    virtual void fadd(const Value &a, const Value &b);
    virtual void sub(const Value &a, const Value &b);
    virtual void fsub(const Value &a, const Value &b);
    virtual void mul(const Value &a, const Value &b);
    virtual void fmul(const Value &a, const Value &b);
    /// Unsigned division
    virtual void udiv(const Value &a, const Value &b);
    /// Signed division.
    virtual void sdiv(const Value &a, const Value &b);
    /// Floating point division.
    virtual void fdiv(const Value &a, const Value &b);
    virtual void urem(const Value &a, const Value &b);
    virtual void srem(const Value &a, const Value &b);
    virtual void frem(const Value &a, const Value &b);
    virtual void shl(const Value &a, const Value &b);
    virtual void lshr(const Value &a, const Value &b);
    virtual void ashr(const Value &a, const Value &b);
    virtual void and_(const Value &a, const Value &b);
    virtual void or_(const Value &a, const Value &b);
    virtual void xor_(const Value &a, const Value &b);
    virtual void icmp(const Value &a, const Value &b,
                      llvm::CmpInst::Predicate predicate);
    virtual void fcmp(const Value &a, const Value &b,
                      llvm::CmpInst::Predicate predicate);
    virtual void trunc(const Value &value);
    virtual void zext(const Value &value);
    virtual void sext(const Value &value);
    virtual void fptrunc(const Value &value);
    virtual void fpext(const Value &value);
    virtual void fptoui(const Value &value);
    virtual void fptosi(const Value &value);
    virtual void uitofp(const Value &value);
    virtual void sitofp(const Value &value);
};

/// Support writing of abstract values to output stream.  Used for
/// logging purposes.
llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
                              const Value &value);

/// @brief
/// Base class for abstract domains with the concept of value accuracy.
class AccuracyValue
{
public:
    /// Get accuracy of the abstract value (0 - 1). In finite-height
    /// lattices, it is determined by the position of the value in the
    /// lattice.
    ///
    /// Accuracy 0 means that the value represents all possible values
    /// (top).  Accuracy 1 means that the value represents the most
    /// precise and exact value (bottom).
    virtual float accuracy() const;

    /// Is it the lowest value.
    virtual bool isBottom() const;
    /// Set to the lowest value.
    virtual void setBottom();

    /// Is it the highest value.
    virtual bool isTop() const;
    /// Set it to the top value of lattice.
    virtual void setTop();
};

/// @brief
/// Base class for abstract domains that can lower the precision and
/// memory requirements on demand.
class VariablePrecisionValue
{
public:
    /// Decrease memory usage of this value below the provided size in
    /// bytes.  Returns true if the memory usage was limited, false when
    /// it was not possible.
    virtual bool limitMemoryUsage(size_t size);
};

} // namespace Canal

#endif // LIBCANAL_VALUE_H
