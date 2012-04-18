#ifndef CANAL_FLOAT_H
#define CANAL_FLOAT_H

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
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;
};

} // namespace Float
} // namespace Canal

#endif // CANAL_FLOAT_H
