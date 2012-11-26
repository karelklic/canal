#ifndef LIBCANAL_DOMAIN_H
#define LIBCANAL_DOMAIN_H

#include "Prereq.h"
#include <cstddef>
#include <string>

namespace Canal {

class Environment;

namespace Widening {
class DataInterface;
} // namespace Widening

/// @brief
/// Base class for all abstract domains.
class Domain
{
public:
    typedef Domain&(Domain::*CastOperation)(const Domain&);

    typedef Domain&(Domain::*BinaryOperation)(const Domain&,
                                              const Domain&);

    typedef Domain&(Domain::*CmpOperation)(const Domain&,
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

    const Environment &getEnvironment() const
    {
        return mEnvironment;
    }

    /// Create a copy of this value.
    virtual Domain *clone() const = 0;

    /// Get memory usage (used byte count) of this abstract value.
    virtual size_t memoryUsage() const = 0;

    /// Create a string representation of the abstract value.
    virtual std::string toString() const = 0;

    /// Set value of this domain to represent zeroed memory.  Needed
    /// for constants with zero initializer.  This can only be called
    /// right after creating a domain.
    virtual void setZero(const llvm::Value *place) = 0;

public: // Lattice
    /// Implementing this is mandatory.  Values are compared while
    /// computing the fixed point.
    virtual bool operator==(const Domain &value) const = 0;

    /// Inequality is implemented by calling the equality operator.
    virtual bool operator!=(const Domain &value) const
    {
        return !operator==(value);
    }

    virtual bool operator<(const Domain &value) const = 0;

    virtual bool operator<=(const Domain &value) const
    {
        return operator==(value) || operator<(value);
    }

    virtual bool operator>(const Domain &value) const = 0;

    virtual bool operator>=(const Domain &value) const
    {
        return operator>(value) || operator==(value);
    }

    /// Merge another value into this one.
    virtual Domain &join(const Domain &value) = 0;

    virtual Domain &meet(const Domain &value) = 0;

    /// Is it the lowest value.
    virtual bool isBottom() const;

    /// Set to the lowest value.
    virtual void setBottom();

    /// Is it the highest value.
    virtual bool isTop() const;

    /// Set it to the top value of lattice.
    virtual void setTop();

    /// Get accuracy of the abstract value (0 - 1). In finite-height
    /// lattices, it is determined by the position of the value in the
    /// lattice.
    ///
    /// Accuracy 0 means that the value represents all possible values
    /// (top).  Accuracy 1 means that the value represents the most
    /// precise and exact value (bottom).
    virtual float accuracy() const;

public: // Instructions operating on values.
    virtual Domain &add(const Domain &a, const Domain &b);

    virtual Domain &fadd(const Domain &a, const Domain &b);

    virtual Domain &sub(const Domain &a, const Domain &b);

    virtual Domain &fsub(const Domain &a, const Domain &b);

    virtual Domain &mul(const Domain &a, const Domain &b);

    virtual Domain &fmul(const Domain &a, const Domain &b);

    /// Unsigned division
    virtual Domain &udiv(const Domain &a, const Domain &b);

    /// Signed division.
    virtual Domain &sdiv(const Domain &a, const Domain &b);

    /// Floating point division.
    virtual Domain &fdiv(const Domain &a, const Domain &b);

    virtual Domain &urem(const Domain &a, const Domain &b);

    virtual Domain &srem(const Domain &a, const Domain &b);

    virtual Domain &frem(const Domain &a, const Domain &b);

    virtual Domain &shl(const Domain &a, const Domain &b);

    virtual Domain &lshr(const Domain &a, const Domain &b);

    virtual Domain &ashr(const Domain &a, const Domain &b);

    virtual Domain &and_(const Domain &a, const Domain &b);

    virtual Domain &or_(const Domain &a, const Domain &b);

    virtual Domain &xor_(const Domain &a, const Domain &b);

    virtual Domain &icmp(const Domain &a, const Domain &b,
                         llvm::CmpInst::Predicate predicate);

    virtual Domain &fcmp(const Domain &a, const Domain &b,
                         llvm::CmpInst::Predicate predicate);

    virtual Domain &trunc(const Domain &value);

    virtual Domain &zext(const Domain &value);

    virtual Domain &sext(const Domain &value);

    virtual Domain &fptrunc(const Domain &value);

    virtual Domain &fpext(const Domain &value);

    virtual Domain &fptoui(const Domain &value);

    virtual Domain &fptosi(const Domain &value);

    virtual Domain &uitofp(const Domain &value);

    virtual Domain &sitofp(const Domain &value);

public: // Widening interface.
    Widening::DataInterface *getWideningData() const
    {
        return mWideningData;
    }

    /// This class takes ownership of the wideningData memory.
    void setWideningData(Widening::DataInterface *wideningData);

public: // Memory layout
    virtual bool isValue() const;

    virtual const llvm::Type &getValueType() const;

    virtual bool hasValueExactSize() const;

    virtual uint64_t getValueExactSize();

    virtual Domain *getValueAbstractSize() const;

    virtual Domain *getValueCell(uint64_t offset) const;

    virtual void mergeValueCell(uint64_t offset, const Domain &value);

private: // Domains are non-copyable.
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Domain &operator=(const Domain &value);
};

} // namespace Canal

#endif // LIBCANAL_DOMAIN_H
