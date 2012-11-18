#ifndef LIBCANAL_DOMAIN_H
#define LIBCANAL_DOMAIN_H

#include "Prereq.h"
#include <cstddef>
#include <string>

namespace Canal {

class State;
class Environment;

namespace Widening {
class DataInterface;
} // namespace Widening

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

    Widening::DataInterface *mWideningData;

public:
    /// Standard constructor.
    Domain(const Environment &environment);

    /// Copy constructor.  Careful!  Copy constructor of base class is
    /// not called by automatically generated copy constructor of
    /// an inherited class.
    Domain(const Domain &value);

    /// Virtual destructor.
    virtual ~Domain();

    const Environment &getEnvironment() const { return mEnvironment; }

    /// Create a copy of this value.
    virtual Domain *clone() const = 0;

    /// Implementing this is mandatory.  Values are compared while
    /// computing the fixed point.
    virtual bool operator==(const Domain &value) const = 0;

    /// Inequality is implemented by calling the equality operator.
    virtual bool operator!=(const Domain &value) const { return !operator==(value); }

    /// Merge another value into this one.
    virtual void merge(const Domain &value) = 0;

    /// Get memory usage (used byte count) of this abstract value.
    virtual size_t memoryUsage() const = 0;

    /// Create a string representation of the abstract value.
    virtual std::string toString() const = 0;

    /// Set value of this domain to represent zeroed memory.  Needed
    /// for constants with zero initializer.
    virtual void setZero(const llvm::Value *place) = 0;

    virtual Widening::DataInterface *getWideningData() const { return mWideningData; }

    /// This class takes ownership of the wideningData memory.
    virtual void setWideningData(Widening::DataInterface *wideningData);

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

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Domain &operator=(const Domain &value);
};


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

#endif // LIBCANAL_DOMAIN_H
