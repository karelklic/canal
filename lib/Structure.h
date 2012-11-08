#ifndef LIBCANAL_STRUCTURE_H
#define LIBCANAL_STRUCTURE_H

#include "Domain.h"
#include "ArrayInterface.h"
#include <vector>

namespace Canal {

class Structure : public Domain, public Array::Interface
{
public:
    std::vector<Domain*> mMembers;

public:
    Structure(const Environment &environment,
              const std::vector<Domain*> &members);

    Structure(const Structure &value);

    virtual ~Structure();

private:
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Structure &operator=(const Structure &value);

public: // Implementation of Domain.
    /// Implementation of Domain::clone().
    /// Covariant return type.
    virtual Structure *clone() const;
    /// Implementation of Domain::cloneCleaned().
    /// Covariant return type.
    virtual Structure *cloneCleaned() const;
    /// Implementation of Domain::operator==().
    virtual bool operator==(const Domain &value) const;
    /// Implementation of Domain::merge().
    virtual void merge(const Domain &value);
    /// Implementation of Domain::memoryUsage().
    virtual size_t memoryUsage() const;
    /// Implementation of Domain::toString().
    virtual std::string toString() const;
    /// Implementation of Domain::setZero().
    virtual void setZero(const llvm::Value *place);

public: // Implementation of Array::Interface.
    /// Implementation of Array::Interface::getItem().
    virtual std::vector<Domain*> getItem(const Domain &offset) const;
    /// Implementation of Array::Interface::getItem().
    virtual Domain *getItem(uint64_t offset) const;
    /// Implementation of Array::Interface::set().
    virtual void setItem(const Domain &offset, const Domain &value);
    /// Implementation of Array::Interface::set().
    virtual void setItem(uint64_t offset, const Domain &value);
};

} // namespace Canal

#endif // LIBCANAL_STRUCTURE_H
