#ifndef CANAL_ABSTRACT_INTEGER_H
#define CANAL_ABSTRACT_INTEGER_H

#include "IntegerBits.h"
//#include "IntegerEnumeration.h"
//#include "IntegerRange.h"

namespace Canal {
namespace Integer {

class Container : public Value
{
public:
    Container(unsigned numBits);
    Container(const Container &container);
    virtual ~Container();

    // Implementation of Value::clone().
    // Covariant return type.
    virtual Container *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;

protected:
    Bits *mBits;
};

} // namespace Integer
} // namespace Canal

#endif
