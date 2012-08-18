#ifndef LIBCANAL_FLOAT_RANGE_H
#define LIBCANAL_FLOAT_RANGE_H

#include "Value.h"
#include <llvm/ADT/APFloat.h>

namespace Canal {
namespace Float {

class Range : public Value, public AccuracyValue
{
public:
    bool mEmpty;
    bool mTop;
    llvm::APFloat mFrom;
    llvm::APFloat mTo;

public:
    Range(const llvm::fltSemantics &semantics);

    // Compares a range with another range.
    // @returns
    //  -1 - one of the ranges is empty
    //   0 - false (predicate not satisfied)
    //   1 - true  (predicate satisfied)
    //   2 - both true and false are possible (top value)
    int compare(const Range &value,
                llvm::CmpInst::Predicate predicate) const;

    const llvm::fltSemantics &getSemantics() const { return mFrom.getSemantics(); }

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Range *clone() const;
    // Implementation of Value::cloneCleaned().
    // Covariant return type.
    virtual Range *cloneCleaned() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString() const;
    // Implementation of Value::matchesString().
    virtual bool matchesString(const std::string &text,
                               std::string &rationale) const;

public: // Implementation of AccuracyValue.
    // Implementation of AccuracyValue::accuracy().
    virtual float accuracy() const;
    // Implementation of AccuracyValue::isBottom().
    virtual bool isBottom() const;
    // Implementation of AccuracyValue::setBottom().
    virtual void setBottom();
    // Implementation of AccuracyValue::isTop().
    virtual bool isTop() const;
    // Implementation of AccuracyValue::setTop().
    virtual void setTop();
};

} // namespace Float
} // namespace Canal

#endif // LIBCANAL_FLOAT_H
