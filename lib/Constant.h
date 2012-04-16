#ifndef CANAL_CONSTANT_H
#define CANAL_CONSTANT_H

#include "Value.h"

namespace llvm {
    class Constant;
}

namespace Canal {

class Constant : public Value
{
public:
    Constant(const llvm::Constant *constant);

    // Implementation of Value::clone().
    // Covariant return type.
    virtual Constant* clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;

protected:
    const llvm::Constant *mConstant;
};

} // namespace Canal

#endif // CANAL_CONSTANT_H
