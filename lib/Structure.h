#ifndef LIBCANAL_STRUCTURE_H
#define LIBCANAL_STRUCTURE_H

#include "Value.h"
#include <vector>

namespace Canal {

class Structure : public Value
{
public:
    std::vector<Value*> mMembers;

public:
    Structure() {};
    Structure(const Structure &structure);
    virtual ~Structure();

    // Implementation of Value::clone().
    // Covariant return type.
    virtual Structure *clone() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString(const State *state) const;
};

} // namespace Canal

#endif // LIBCANAL_STRUCTURE_H
