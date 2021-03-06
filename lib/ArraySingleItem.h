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
    /// @brief
    ///   Abstract value of the array members.
    ///
    /// Abstract value representing all concrete values of the array.
    /// It is never NULL.
    Domain *mValue;

    /// @brief
    ///   Abstract number of the elements in the array.
    ///
    /// Abstract value representing the number of elements in the
    /// array.  It is never NULL.
    Domain *mSize;

    /// @brief
    ///   Type of the array.
    ///
    /// Type of the concrete array represented by this abstract
    /// domain.
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

    virtual Domain *extractelement(const Domain &index) const;

    virtual SingleItem &insertelement(const Domain &array,
                                      const Domain &element,
                                      const Domain &index);

    virtual SingleItem &shufflevector(const Domain &a,
                                      const Domain &b,
                                      const std::vector<uint32_t> &mask);

    virtual Domain *extractvalue(const std::vector<unsigned> &indices) const;

    virtual SingleItem &insertvalue(const Domain &aggregate,
                                    const Domain &element,
                                    const std::vector<unsigned> &indices);

    virtual void insertvalue(const Domain &element,
                             const std::vector<unsigned> &indices);

    virtual Domain *load(const llvm::Type &type,
                         const std::vector<Domain*> &offsets) const;

    virtual SingleItem &store(const Domain &value,
                              const std::vector<Domain*> &offsets,
                              bool overwrite);

    virtual const llvm::SequentialType &getValueType() const { return mType; }
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_SINGLE_ITEM_H
