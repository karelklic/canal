#ifndef LIBCANAL_STRUCTURE_H
#define LIBCANAL_STRUCTURE_H

#include "Domain.h"
#include <vector>

namespace Canal {

class Structure : public Domain
{
public:
    std::vector<Domain*> mMembers;

    const llvm::StructType &mType;

public:
    Structure(const Environment &environment,
              const llvm::StructType &type);

    Structure(const Environment &environment,
              const llvm::StructType &type,
              const std::vector<Domain*> &members);

    Structure(const Structure &value);

    virtual ~Structure();

    static bool classof(const Domain *value)
    {
        return value->getKind() == StructureKind;
    }

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Structure *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual Structure &join(const Domain &value);

    virtual Structure &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual Structure &insertelement(const Domain &array,
                                     const Domain &element,
                                     const Domain &index);

    virtual std::vector<Domain*> getItem(const Domain &offset) const;

    virtual Domain *getItem(uint64_t offset) const;
};

} // namespace Canal

#endif // LIBCANAL_STRUCTURE_H
