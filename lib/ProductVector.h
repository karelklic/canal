#ifndef LIBCANAL_PRODUCT_VECTOR_H
#define LIBCANAL_PRODUCT_VECTOR_H

#include "Domain.h"

namespace Canal {
namespace Product {

class Vector : public Domain
{
public:
    std::vector<Domain*> mValues;

public:
    Vector(const Environment &environment);

    /// Copy constructor.  Creates independent copy of the container.
    Vector(const Vector &value);

    /// Destructor.  Deletes the contents of the container.
    virtual ~Vector();

    static bool classof(const Domain *value)
    {
        return value->getKind() == ProductVectorKind;
    }

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Vector *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual Vector &join(const Domain &value);

    virtual Vector &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual Vector &add(const Domain &a, const Domain &b);

    virtual Vector &sub(const Domain &a, const Domain &b);

    virtual Vector &mul(const Domain &a, const Domain &b);

    virtual Vector &udiv(const Domain &a, const Domain &b);

    virtual Vector &sdiv(const Domain &a, const Domain &b);

    virtual Vector &urem(const Domain &a, const Domain &b);

    virtual Vector &srem(const Domain &a, const Domain &b);

    virtual Vector &shl(const Domain &a, const Domain &b);

    virtual Vector &lshr(const Domain &a, const Domain &b);

    virtual Vector &ashr(const Domain &a, const Domain &b);

    virtual Vector &and_(const Domain &a, const Domain &b);

    virtual Vector &or_(const Domain &a, const Domain &b);

    virtual Vector &xor_(const Domain &a, const Domain &b);

    virtual Vector &icmp(const Domain &a, const Domain &b,
                         llvm::CmpInst::Predicate predicate);

    virtual Vector &fcmp(const Domain &a, const Domain &b,
                         llvm::CmpInst::Predicate predicate);

    virtual Vector &trunc(const Domain &value);

    virtual Vector &zext(const Domain &value);

    virtual Vector &sext(const Domain &value);

    virtual Vector &fptoui(const Domain &value);

    virtual Vector &fptosi(const Domain &value);

    virtual Domain *extractelement(const Domain &index) const;

    virtual Vector &insertelement(const Domain &array,
                                  const Domain &element,
                                  const Domain &index);

    virtual Vector &shufflevector(const Domain &a,
                                  const Domain &b,
                                  const std::vector<uint32_t> &mask);

    virtual Domain *extractvalue(const std::vector<unsigned> &indices) const;

    virtual Vector &insertvalue(const Domain &aggregate,
                                const Domain &element,
                                const std::vector<unsigned> &indices);

    virtual void insertvalue(const Domain &element,
                             const std::vector<unsigned> &indices);

    virtual const llvm::Type &getValueType() const;

    virtual Domain *loadValue(const llvm::Type &type,
                              const Domain &offset) const;

    virtual void storeValue(const Domain &value,
                            const Domain &offset,
                            bool isSingleTarget);

public: // Reduced Product
    /// Initiate communication between contained Domains
    /// to enhance their accuracy.
    void collaborate();
};

} // namespace Product
} // namespace Canal

#endif // LIBCANAL_PRODUCT_VECTOR_H
