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
    const llvm::Constant *mConstant;

public:
    Constant(Environment &environment, const llvm::Constant *constant);

    // Checks if this constant is an integer number.
    bool isAPInt() const;
    // Returns number for integer constant.
    const llvm::APInt &getAPInt() const;

    bool isGetElementPtr() const;

    // Creates a modifiable value from the constant.  Caller is
    // responsible for releasing its memory.
    Value *toModifiableValue() const;

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Constant *clone() const;
    // Implementation of Value::cloneCleaned().
    // Covariant return type.
    virtual Constant *cloneCleaned() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString() const;
    // Implementation of Value::matchesString().
    virtual bool matchesString(const std::string &text,
                               std::string &rationale) const;
    virtual void merge(const Value &value);
};

} // namespace Canal

#endif // LIBCANAL_CONSTANT_H
