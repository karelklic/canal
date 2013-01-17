#ifndef LIBCANAL_INTEGER_CONTAINER_H
#define LIBCANAL_INTEGER_CONTAINER_H

#include "Domain.h"

namespace Canal {
namespace Integer {

class Bitfield;
class Set;
class Interval;

class Container : public Domain
{
public:
    std::vector<Domain*> mValues;

public:
    Container(const Environment &environment);

    /// Copy constructor.  Creates independent copy of the container.
    Container(const Container &value);

    /// Destructor.  Deletes the contents of the container.
    virtual ~Container();

    static bool classof(const Domain *value)
    {
        return value->getKind() == IntegerContainerKind;
    }

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Container *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual Container &join(const Domain &value);

    virtual Container &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual Container &add(const Domain &a, const Domain &b);

    virtual Container &sub(const Domain &a, const Domain &b);

    virtual Container &mul(const Domain &a, const Domain &b);

    virtual Container &udiv(const Domain &a, const Domain &b);

    virtual Container &sdiv(const Domain &a, const Domain &b);

    virtual Container &urem(const Domain &a, const Domain &b);

    virtual Container &srem(const Domain &a, const Domain &b);

    virtual Container &shl(const Domain &a, const Domain &b);

    virtual Container &lshr(const Domain &a, const Domain &b);

    virtual Container &ashr(const Domain &a, const Domain &b);

    virtual Container &and_(const Domain &a, const Domain &b);

    virtual Container &or_(const Domain &a, const Domain &b);

    virtual Container &xor_(const Domain &a, const Domain &b);

    virtual Container &icmp(const Domain &a, const Domain &b,
                            llvm::CmpInst::Predicate predicate);

    virtual Container &fcmp(const Domain &a, const Domain &b,
                            llvm::CmpInst::Predicate predicate);

    virtual Container &trunc(const Domain &value);

    virtual Container &zext(const Domain &value);

    virtual Container &sext(const Domain &value);

    virtual Container &fptoui(const Domain &value);

    virtual Container &fptosi(const Domain &value);

    virtual Domain *extractelement(const Domain &index) const;

    virtual Container &insertelement(const Domain &array,
                                     const Domain &element,
                                     const Domain &index);

    virtual Container &shufflevector(const Domain &a,
                                     const Domain &b,
                                     const std::vector<uint32_t> &mask);

    virtual Domain *extractvalue(const std::vector<unsigned> &indices) const;

    virtual Container &insertvalue(const Domain &aggregate,
                                   const Domain &element,
                                   const std::vector<unsigned> &indices);

    virtual void insertvalue(const Domain &element,
                             const std::vector<unsigned> &indices);

    virtual Domain *load(const llvm::Type &type,
                         const std::vector<Domain*> &offsets) const;

    virtual Container &store(const Domain &value,
                             const std::vector<Domain*> &offsets,
                             bool overwrite);

    virtual const llvm::Type &getValueType() const;
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_H
