#ifndef LIBCANAL_ARRAY_STRING_PREFIX_H
#define LIBCANAL_ARRAY_STRING_PREFIX_H

#include "Domain.h"
#include "ArrayInterface.h"

namespace Canal {
namespace Array {

/// Array with exact size and limited length.  It keeps all array
/// members separately, not losing precision at all.
class StringPrefix : public Domain, public Interface
{
public:
    std::string mPrefix;
    bool mIsBottom;

    const llvm::SequentialType &mType;

public:
    /// Standard constructor.
    StringPrefix(const Environment &environment,
                 const llvm::SequentialType &type);

    /// @param values
    ///   This class takes ownership of the values.
    StringPrefix(const Environment &environment,
                 const llvm::SequentialType &type,
                 const std::vector<Domain*> &values);

    StringPrefix(const Environment &environment,
                 const llvm::SequentialType &type,
                 Domain *size);

    StringPrefix(const Environment &environment,
                 const std::string &value);

public: // Implementation of Domain.
    /// Covariant return type.
    virtual StringPrefix *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual bool operator>(const Domain &value) const;

    virtual StringPrefix &join(const Domain &value);

    virtual StringPrefix &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual StringPrefix &add(const Domain &a, const Domain &b);

    virtual StringPrefix &fadd(const Domain &a, const Domain &b);

    virtual StringPrefix &sub(const Domain &a, const Domain &b);

    virtual StringPrefix &fsub(const Domain &a, const Domain &b);

    virtual StringPrefix &mul(const Domain &a, const Domain &b);

    virtual StringPrefix &fmul(const Domain &a, const Domain &b);

    virtual StringPrefix &udiv(const Domain &a, const Domain &b);

    virtual StringPrefix &sdiv(const Domain &a, const Domain &b);

    virtual StringPrefix &fdiv(const Domain &a, const Domain &b);

    virtual StringPrefix &urem(const Domain &a, const Domain &b);

    virtual StringPrefix &srem(const Domain &a, const Domain &b);

    virtual StringPrefix &frem(const Domain &a, const Domain &b);

    virtual StringPrefix &shl(const Domain &a, const Domain &b);

    virtual StringPrefix &lshr(const Domain &a, const Domain &b);

    virtual StringPrefix &ashr(const Domain &a, const Domain &b);

    virtual StringPrefix &and_(const Domain &a, const Domain &b);

    virtual StringPrefix &or_(const Domain &a, const Domain &b);

    virtual StringPrefix &xor_(const Domain &a, const Domain &b);

    virtual StringPrefix &icmp(const Domain &a, const Domain &b,
                               llvm::CmpInst::Predicate predicate);

    virtual StringPrefix &fcmp(const Domain &a, const Domain &b,
                               llvm::CmpInst::Predicate predicate);

    virtual bool isValue() const
    {
        return true;
    }

    /// Covariant return type.
    virtual const llvm::SequentialType &getValueType() const;

    virtual bool hasValueExactSize() const
    {
        return false;
    }

    virtual Domain *getValueAbstractSize() const;

    virtual Domain *getValueCell(uint64_t offset) const;

    virtual void mergeValueCell(uint64_t offset, const Domain &value);

public: // Implementation of Array::Interface.
    virtual std::vector<Domain*> getItem(const Domain &offset) const;

    virtual Domain *getItem(uint64_t offset) const;

    virtual void setItem(const Domain &offset, const Domain &value);

    virtual void setItem(uint64_t offset, const Domain &value);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_STRING_PREFIX_H
