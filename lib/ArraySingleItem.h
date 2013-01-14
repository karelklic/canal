#ifndef LIBCANAL_ARRAY_SINGLE_ITEM_H
#define LIBCANAL_ARRAY_SINGLE_ITEM_H

#include "Domain.h"

namespace Canal {
namespace Array {

/// The most trivial array type.  It treats all array members as a
/// single value.  This means all the operations on the array are
/// merged and used to move the single value up in its lattice.
//
/// This array type is very imprecise.
class SingleItem : public Domain
{
public:
    Domain *mValue;

    /// Number of elements in the array.
    /// It is either a Constant or Integer::Container.
    Domain *mSize;

    const llvm::SequentialType &mType;

public:
    SingleItem(const Environment &environment,
               const llvm::SequentialType &type);

    SingleItem(const Environment &environment,
               const llvm::SequentialType &type,
               std::vector<Domain*>::const_iterator begin,
               std::vector<Domain*>::const_iterator end);

    SingleItem(const Environment &environment,
               const llvm::SequentialType &type,
               Domain *size);

    SingleItem(const SingleItem &value);

    virtual ~SingleItem();

    static bool classof(const Domain *value)
    {
        return value->getKind() == ArraySingleItemKind;
    }

public: // Implementation of Domain.
    /// Covariant return type.
    virtual SingleItem *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    /// Computes a join of both values and array sizes.
    virtual SingleItem &join(const Domain &value);

    /// Computes a meet of both values and array sizes.
    virtual SingleItem &meet(const Domain &value);

    /// Check if all items in the array are bottom.
    virtual bool isBottom() const;

    /// Set all items in the array to bottom.
    virtual void setBottom();

    /// Check if all items in the array are top.
    virtual bool isTop() const;

    /// Set all items in the array to top.
    virtual void setTop();

    virtual float accuracy() const;

    virtual SingleItem &add(const Domain &a, const Domain &b);

    virtual SingleItem &fadd(const Domain &a, const Domain &b);

    virtual SingleItem &sub(const Domain &a, const Domain &b);

    virtual SingleItem &fsub(const Domain &a, const Domain &b);

    virtual SingleItem &mul(const Domain &a, const Domain &b);

    virtual SingleItem &fmul(const Domain &a, const Domain &b);

    virtual SingleItem &udiv(const Domain &a, const Domain &b);

    virtual SingleItem &sdiv(const Domain &a, const Domain &b);

    virtual SingleItem &fdiv(const Domain &a, const Domain &b);

    virtual SingleItem &urem(const Domain &a, const Domain &b);

    virtual SingleItem &srem(const Domain &a, const Domain &b);

    virtual SingleItem &frem(const Domain &a, const Domain &b);

    virtual SingleItem &shl(const Domain &a, const Domain &b);

    virtual SingleItem &lshr(const Domain &a, const Domain &b);

    virtual SingleItem &ashr(const Domain &a, const Domain &b);

    virtual SingleItem &and_(const Domain &a, const Domain &b);

    virtual SingleItem &or_(const Domain &a, const Domain &b);

    virtual SingleItem &xor_(const Domain &a, const Domain &b);

    virtual SingleItem &icmp(const Domain &a, const Domain &b,
                             llvm::CmpInst::Predicate predicate);

    virtual SingleItem &fcmp(const Domain &a, const Domain &b,
                             llvm::CmpInst::Predicate predicate);

    virtual std::vector<Domain*> getItem(const Domain &offset) const;

    virtual Domain *getItem(uint64_t offset) const;

    virtual void setItem(const Domain &offset, const Domain &value);

    virtual void setItem(uint64_t offset, const Domain &value);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_SINGLE_ITEM_H
