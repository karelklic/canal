#ifndef LIBCANAL_FLOAT_INTERVAL_H
#define LIBCANAL_FLOAT_INTERVAL_H

#include "Domain.h"
#include <llvm/ADT/APFloat.h>

namespace Canal {
namespace Float {

class Interval : public Domain, public AccuracyDomain
{
public:
    bool mEmpty;
    bool mTop;
    llvm::APFloat mFrom;
    llvm::APFloat mTo;

public:
    /// For super_ptr
    typedef Interval type;

    Interval(const Environment &environment,
             const llvm::fltSemantics &semantics);

    Interval(const Environment &environment,
             const llvm::APFloat &number);

    /// Copy constructor.
    Interval(const Interval &value);

    // Compares a interval with another interval.
    // @returns
    //  -1 - one of the intervals is empty
    //   0 - false (predicate not satisfied)
    //   1 - true  (predicate satisfied)
    //   2 - both true and false are possible (top value)
    int compare(const Interval &value,
                llvm::CmpInst::Predicate predicate) const;

    bool isNaN() const;

    const llvm::fltSemantics &getSemantics() const;

    bool isSingleValue() const;

    bool intersects(const Interval &value) const;

    llvm::APFloat getMax() const;

    llvm::APFloat getMin() const;

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Interval &operator=(const Interval &value);

public: // Implementation of Domain.
    // Implementation of Domain::clone().
    // Covariant return type.
    virtual Interval *clone() const;
    // Implementation of Domain::cloneCleaned().
    // Covariant return type.
    virtual Interval *cloneCleaned() const;
    // Implementation of Domain::operator==().
    virtual bool operator==(const Domain& value) const;
    // Implementation of Domain::merge().
    virtual void merge(const Domain &value);
    // Implementation of Domain::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Domain::toString().
    virtual std::string toString() const;
    /// Implementation of Domain::setZero().
    virtual void setZero(const llvm::Value *place);

public: // Implementation of AccuracyDomain.
    // Implementation of AccuracyDomain::accuracy().
    virtual float accuracy() const;
    // Implementation of AccuracyDomain::isBottom().
    virtual bool isBottom() const;
    // Implementation of AccuracyDomain::setBottom().
    virtual void setBottom();
    // Implementation of AccuracyDomain::isTop().
    virtual bool isTop() const;
    // Implementation of AccuracyDomain::setTop().
    virtual void setTop();

    virtual void fadd(const Domain &a, const Domain &b);
    virtual void fsub(const Domain &a, const Domain &b);
    virtual void fmul(const Domain &a, const Domain &b);
    virtual void fdiv(const Domain &a, const Domain &b);
    virtual void frem(const Domain &a, const Domain &b);
    virtual void uitofp(const Domain &value);
    virtual void sitofp(const Domain &value);
};

} // namespace Float
} // namespace Canal

#endif // LIBCANAL_FLOAT_H
