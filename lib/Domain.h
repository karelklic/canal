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
class Environment;

/// @brief
/// Base class for all abstract domains.
class Domain
{
public:
    typedef void(Domain::*CastOperation)(const Domain&);

    typedef void(Domain::*BinaryOperation)(const Domain&,
                                          const Domain&);

    typedef void(Domain::*CmpOperation)(const Domain&,
                                       const Domain&,
                                       llvm::CmpInst::Predicate predicate);

    const Environment &mEnvironment;

public:
    /// Standard constructor.
    Domain(const Environment &environment);
    /// Virtual destructor.
    virtual ~Domain() {};

    /// Create a copy of this value.
    virtual Domain *clone() const = 0;

    /// This is used to obtain instance of the value type and to get an
    /// empty value at the same time.
    virtual Domain *cloneCleaned() const = 0;

    /// Implementing this is mandatory.  Values are compared while
    /// computing the fixed point.
    virtual bool operator==(const Domain &value) const = 0;
    /// Inequality is implemented by calling the equality operator.
    virtual bool operator!=(const Domain &value) const;

    /// Merge another value into this one.
    virtual void merge(const Domain &value);

    /// Get memory usage (used byte count) of this abstract value.
    virtual size_t memoryUsage() const = 0;

    /// An idea for different memory interpretation.
    /// virtual Domain *castTo(const llvm::Type *itemType, int offset) const = 0;

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

    /// Set value of this domain to zero (needed for zeroinitializer)
    virtual void setZero() = 0;

public:
    /// Implementation of instructions operating on values.
    virtual void add(const Domain &a, const Domain &b);
    virtual void fadd(const Domain &a, const Domain &b);
    virtual void sub(const Domain &a, const Domain &b);
    virtual void fsub(const Domain &a, const Domain &b);
    virtual void mul(const Domain &a, const Domain &b);
    virtual void fmul(const Domain &a, const Domain &b);
    /// Unsigned division
    virtual void udiv(const Domain &a, const Domain &b);
    /// Signed division.
    virtual void sdiv(const Domain &a, const Domain &b);
    /// Floating point division.
    virtual void fdiv(const Domain &a, const Domain &b);
    virtual void urem(const Domain &a, const Domain &b);
    virtual void srem(const Domain &a, const Domain &b);
    virtual void frem(const Domain &a, const Domain &b);
    virtual void shl(const Domain &a, const Domain &b);
    virtual void lshr(const Domain &a, const Domain &b);
    virtual void ashr(const Domain &a, const Domain &b);
    virtual void and_(const Domain &a, const Domain &b);
    virtual void or_(const Domain &a, const Domain &b);
    virtual void xor_(const Domain &a, const Domain &b);
    virtual void icmp(const Domain &a, const Domain &b,
                      llvm::CmpInst::Predicate predicate);
    virtual void fcmp(const Domain &a, const Domain &b,
                      llvm::CmpInst::Predicate predicate);
    virtual void trunc(const Domain &value);
    virtual void zext(const Domain &value);
    virtual void sext(const Domain &value);
    virtual void fptrunc(const Domain &value);
    virtual void fpext(const Domain &value);
    virtual void fptoui(const Domain &value);
    virtual void fptosi(const Domain &value);
    virtual void uitofp(const Domain &value);
    virtual void sitofp(const Domain &value);
};

/// Support writing of abstract values to output stream.  Used for
/// logging purposes.
llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
                              const Domain &value);

/// @brief
/// Base class for abstract domains with the concept of value accuracy.
class AccuracyDomain
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
class VariablePrecisionDomain
{
public:
    /// Decrease memory usage of this value below the provided size in
    /// bytes.  Returns true if the memory usage was limited, false when
    /// it was not possible.
    virtual bool limitMemoryUsage(size_t size);
};

} // namespace Canal

#endif // LIBCANAL_VALUE_H
