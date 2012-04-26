#ifndef LIBCANAL_CONSTANT_H
#define LIBCANAL_CONSTANT_H

#include "Value.h"

namespace llvm {
    class Constant;
    class APInt;
}

namespace Canal {

class Value;

class Constant : public Value
{
public:
    Constant(const llvm::Constant *constant = NULL);

    // Implementation of Value::clone().
    // Covariant return type.
    virtual Constant* clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &ostream) const;

    // Checks if this constant is an integer number.
    bool isAPInt() const;
    // Returns number for integer constant.
    const llvm::APInt &getAPInt() const;

    // Creates a modifiable value from the constant.  Caller is
    // responsible for releasing its memory.
    Value *toModifiableValue() const;

public:
    const llvm::Constant *mConstant;
};

} // namespace Canal

#endif // LIBCANAL_CONSTANT_H
