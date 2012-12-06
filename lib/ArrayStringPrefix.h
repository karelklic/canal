#ifndef LIBCANAL_ARRAY_STRING_PREFIX_H
#define LIBCANAL_ARRAY_STRING_PREFIX_H

#include "Domain.h"

namespace Canal {
namespace Array {

class StringPrefix : public Domain
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
                 std::vector<Domain*>::const_iterator begin,
                 std::vector<Domain*>::const_iterator end);

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

    virtual StringPrefix &shufflevector(const Domain &v1,
                                        const Domain &v2,
                                        const std::vector<uint32_t> &mask);

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

    virtual Domain *getValueCell(uint64_t offset) const;

    virtual void mergeValueCell(uint64_t offset, const Domain &value);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_STRING_PREFIX_H
