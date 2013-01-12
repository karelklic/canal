#ifndef LIBCANAL_FLOAT_INTERVAL_H
#define LIBCANAL_FLOAT_INTERVAL_H

#include "Domain.h"

namespace Canal {
namespace Float {

class Interval : public Domain
{
    bool mEmpty;
    bool mTop;
    llvm::APFloat mFrom;
    llvm::APFloat mTo;

public:
    Interval(const Environment &environment,
             const llvm::fltSemantics &semantics);

    Interval(const Environment &environment,
             const llvm::APFloat &constant);

    Interval(const Environment &environment,
             const llvm::APFloat &from,
             const llvm::APFloat &to);

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

    bool isConstant() const;

    bool intersects(const Interval &value) const;

    llvm::APFloat getMax() const;

    llvm::APFloat getMin() const;

    static bool classof(const Domain *value)
    {
        return value->getKind() == FloatIntervalKind;
    }

public: // Implementation of Domain.
    // Covariant return type.
    virtual Interval *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain& value) const;

    virtual bool operator<(const Domain &value) const;

    virtual bool operator>(const Domain &value) const;

    virtual Interval &join(const Domain &value);

    virtual Interval &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual Interval &fadd(const Domain &a, const Domain &b);

    virtual Interval &fsub(const Domain &a, const Domain &b);

    virtual Interval &fmul(const Domain &a, const Domain &b);

    virtual Interval &fdiv(const Domain &a, const Domain &b);

    virtual Interval &frem(const Domain &a, const Domain &b);

    virtual Interval &uitofp(const Domain &value);

    virtual Interval &sitofp(const Domain &value);
};

} // namespace Float
} // namespace Canal

#endif // LIBCANAL_FLOAT_INTERVAL_H
