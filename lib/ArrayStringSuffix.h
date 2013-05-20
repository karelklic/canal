#ifndef LIBCANAL_ARRAY_STRING_SUFFIX_H
#define LIBCANAL_ARRAY_STRING_SUFFIX_H

#include "Domain.h"

namespace Canal {
namespace Array {

class StringSuffix : public Domain
{
public:
    bool mIsBottom;

    std::string mSuffix;

    const llvm::SequentialType &mType;

public:
    StringSuffix(const Environment &environment,
                 const llvm::SequentialType &type);

    StringSuffix(const Environment &environment,
                 const llvm::SequentialType &type,
                 std::vector<Domain *>::const_iterator begin,
                 std::vector<Domain *>::const_iterator end);

    StringSuffix(const Environment &environment,
                 const std::string &value);

    static bool classof(const Domain *value)
    {
        return value->getKind() == ArrayStringSuffixKind;
    }

    void strcat(const StringSuffix &source);

public:
    virtual StringSuffix *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual StringSuffix &join(const Domain &value);

    virtual StringSuffix &meet(const Domain &value);

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual float accuracy() const;

    virtual StringSuffix &add(const Domain &a, const Domain &b);

    virtual StringSuffix &fadd(const Domain &a, const Domain &b);

    virtual StringSuffix &sub(const Domain &a, const Domain &b);

    virtual StringSuffix &fsub(const Domain &a, const Domain &b);

    virtual StringSuffix &mul(const Domain &a, const Domain &b);

    virtual StringSuffix &fmul(const Domain &a, const Domain &b);

    virtual StringSuffix &udiv(const Domain &a, const Domain &b);

    virtual StringSuffix &sdiv(const Domain &a, const Domain &b);

    virtual StringSuffix &fdiv(const Domain &a, const Domain &b);

    virtual StringSuffix &urem(const Domain &a, const Domain &b);

    virtual StringSuffix &srem(const Domain &a, const Domain &b);

    virtual StringSuffix &frem(const Domain &a, const Domain &b);

    virtual StringSuffix &shl(const Domain &a, const Domain &b);

    virtual StringSuffix &lshr(const Domain &a, const Domain &b);

    virtual StringSuffix &ashr(const Domain &a, const Domain &b);

    virtual StringSuffix &and_(const Domain &a, const Domain &b);

    virtual StringSuffix &or_(const Domain &a, const Domain &b);

    virtual StringSuffix &xor_(const Domain &a, const Domain &b);

    virtual StringSuffix &icmp(const Domain &a,
                               const Domain &b,
                               llvm::CmpInst::Predicate predicate);

    virtual StringSuffix &fcmp(const Domain &a,
                               const Domain &b,
                               llvm::CmpInst::Predicate predicate);

    virtual Domain *extractelement(const Domain &index) const;

    virtual StringSuffix &insertelement(const Domain &array,
                                        const Domain &element,
                                        const Domain &index);

    virtual StringSuffix &shufflevector(const Domain &a,
                                        const Domain &b,
                                        const std::vector<uint32_t> &mask);

    virtual Domain *extractvalue(const std::vector<unsigned> &indices) const;

    virtual StringSuffix &insertvalue(const Domain &aggregate,
                                      const Domain &element,
                                      const std::vector<unsigned> &indices);

    virtual void insertvalue(const Domain &element,
                             const std::vector<unsigned> &indices);

    virtual Domain *load(const llvm::Type &type,
                         const std::vector<Domain*> &offsets) const;

    virtual StringSuffix &store(const Domain &value,
                                const std::vector<Domain*> &offsets,
                                bool overwrite);

    virtual const llvm::SequentialType &getValueType() const { return mType; }

};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_STRING_SUFFIX_H

