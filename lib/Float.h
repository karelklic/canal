#ifndef LIBCANAL_FLOAT_H
#define LIBCANAL_FLOAT_H

#include "Value.h"
#include <llvm/ADT/APFloat.h>

namespace Canal {
namespace Float {

class Range : public Value
{
public:
    llvm::APFloat mFrom;
    llvm::APFloat mTo;

public:
    Range(const llvm::fltSemantics &semantics);

    // Implementation of Value::clone().
    // Covariant return type.
    virtual Range *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value& value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;
};

} // namespace Float
} // namespace Canal

#endif // LIBCANAL_FLOAT_H
