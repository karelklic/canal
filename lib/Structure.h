#ifndef LIBCANAL_STRUCTURE_H
#define LIBCANAL_STRUCTURE_H

#include "Value.h"
#include "ArrayInterface.h"
#include <vector>

namespace Canal {

class Structure : public Value, public Array::Interface
{
public:
    std::vector<Value*> mMembers;

public:
    Structure() {};
    Structure(const Structure &structure);
    virtual ~Structure();

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual Structure *clone() const;
    // Implementation of Value::cloneCleaned().
    // Covariant return type.
    virtual Structure *cloneCleaned() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString() const;

public: // Implementation of Array::Interface.
    // Implementation of Array::Interface::getItem().
    virtual std::vector<Value*> getItem(const Value &offset) const;
    // Implementation of Array::Interface::getItem().
    virtual Value *getItem(uint64_t offset) const;
    // Implementation of Array::Interface::set().
    virtual void setItem(const Value &offset, const Value &value);
    // Implementation of Array::Interface::set().
    virtual void setItem(uint64_t offset, const Value &value);
};

} // namespace Canal

#endif // LIBCANAL_STRUCTURE_H
