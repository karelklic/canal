#ifndef LIBCANAL_FLOAT_RANGE_H
#define LIBCANAL_FLOAT_RANGE_H

#include "Domain.h"
#include <llvm/ADT/APFloat.h>

namespace Canal {
namespace Float {

class Range : public Domain, public AccuracyDomain
{
public:
    bool mEmpty;
    bool mTop;
    llvm::APFloat mFrom;
    llvm::APFloat mTo;

public:
    Range(const Environment &environment,
          const llvm::fltSemantics &semantics);

    // Compares a range with another range.
    // @returns
    //  -1 - one of the ranges is empty
    //   0 - false (predicate not satisfied)
    //   1 - true  (predicate satisfied)
    //   2 - both true and false are possible (top value)
    int compare(const Range &value,
                llvm::CmpInst::Predicate predicate) const;
    bool isNaN() const;

    const llvm::fltSemantics &getSemantics() const { return mFrom.getSemantics(); }
    bool isSingleValue() const;
    bool intersects(const Range &value) const;
    llvm::APFloat getMax() const;
    llvm::APFloat getMin() const;

public: // Implementation of Domain.
    // Implementation of Domain::clone().
    // Covariant return type.
    virtual Range *clone() const;
    // Implementation of Domain::cloneCleaned().
    // Covariant return type.
    virtual Range *cloneCleaned() const;
    // Implementation of Domain::operator==().
    virtual bool operator==(const Domain& value) const;
    // Implementation of Domain::merge().
    virtual void merge(const Domain &value);
    // Implementation of Domain::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Domain::toString().
    virtual std::string toString() const;
    // Implementation of Domain::matchesString().
    virtual bool matchesString(const std::string &text,
                               std::string &rationale) const;

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
};

} // namespace Float
} // namespace Canal

#endif // LIBCANAL_FLOAT_H
