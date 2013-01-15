#ifndef LIBCANAL_ARRAY_STRING_PREFIX_H
#define LIBCANAL_ARRAY_STRING_PREFIX_H

#include "Domain.h"

namespace Canal {
namespace Array {

class StringPrefix : public Domain
{
public:
    std::string mPrefix;
    //std::vector<Domain*> mPrefix;

    bool mIsBottom;

    const llvm::SequentialType &mType;

public:
    /// Standard constructor
    StringPrefix(const Environment &environment,
                 const llvm::SequentialType &type);

    /// @param values
    /// This class takes ownership of the values.
    StringPrefix(const Environment &environment,
                 const llvm::SequentialType &type,
                 std::vector<Domain*>::const_iterator begin,
                 std::vector<Domain*>::const_iterator end);

    /// @param value
    ///   This class does not take ownership of this value.
    StringPrefix(const Environment &environment,
                 const std::string &value);

    static bool classof(const Domain *value)
    {
        return value->getKind() == ArrayStringPrefixKind;
    }

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

    virtual StringPrefix &icmp(const Domain &a,
                               const Domain &b,
                               llvm::CmpInst::Predicate predicate);

    virtual StringPrefix &fcmp(const Domain &a,
                               const Domain &b,
                               llvm::CmpInst::Predicate predicate);

    virtual Domain *extractelement(const Domain &index) const;

    virtual StringPrefix &insertelement(const Domain &array,
                                        const Domain &element,
                                        const Domain &index);

    virtual StringPrefix &shufflevector(const Domain &a,
                                        const Domain &b,
                                        const std::vector<uint32_t> &mask);

    virtual Domain *extractvalue(const std::vector<unsigned> &indices) const;

    virtual StringPrefix &insertvalue(const Domain &aggregate,
                                      const Domain &element,
                                      const std::vector<unsigned> &indices);

    virtual void insertvalue(const Domain &element,
                             const std::vector<unsigned> &indices);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_STRING_PREFIX_H
