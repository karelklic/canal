#ifndef LIBCANAL_ARRAY_EXACT_SIZE_H
#define LIBCANAL_ARRAY_EXACT_SIZE_H

#include "Domain.h"
#include "ArrayInterface.h"

namespace Canal {
namespace Array {

/// Array with exact size and limited length.  It keeps all array
/// members separately, not losing precision at all.
class ExactSize : public Domain, public Interface
{
public:
    std::vector<Domain*> mValues;

public:
    /// @param value
    ///   This class does not take ownership of this value.
    ExactSize(const Environment &environment,
              const uint64_t size,
              const Domain &value);

    /// @param values
    ///   This class takes ownership of the values.
    ExactSize(const Environment &environment,
              const std::vector<Domain*> &values);

    /// Copy constructor.
    ExactSize(const ExactSize &value);

    // Standard destructor.
    virtual ~ExactSize();

    size_t size() const
    {
        return mValues.size();
    }

    static bool classof(const Domain *value)
    {
        return value->getKind() == ArrayExactSizeKind;
    }

public: // Implementation of Domain.
    /// Covariant return type.
    virtual ExactSize *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual bool operator>(const Domain &value) const;

    virtual ExactSize &join(const Domain &value);

    virtual ExactSize &meet(const Domain &value);

    /// Check if all items in the array are bottom.
    virtual bool isBottom() const;

    /// Set all items in the array to bottom.
    virtual void setBottom();

    /// Check if all items in the array are top.
    virtual bool isTop() const;

    /// Set all items in the array to top.
    virtual void setTop();

    virtual float accuracy() const;

    virtual ExactSize &add(const Domain &a, const Domain &b);

    virtual ExactSize &fadd(const Domain &a, const Domain &b);

    virtual ExactSize &sub(const Domain &a, const Domain &b);

    virtual ExactSize &fsub(const Domain &a, const Domain &b);

    virtual ExactSize &mul(const Domain &a, const Domain &b);

    virtual ExactSize &fmul(const Domain &a, const Domain &b);

    virtual ExactSize &udiv(const Domain &a, const Domain &b);

    virtual ExactSize &sdiv(const Domain &a, const Domain &b);

    virtual ExactSize &fdiv(const Domain &a, const Domain &b);

    virtual ExactSize &urem(const Domain &a, const Domain &b);

    virtual ExactSize &srem(const Domain &a, const Domain &b);

    virtual ExactSize &frem(const Domain &a, const Domain &b);

    virtual ExactSize &shl(const Domain &a, const Domain &b);

    virtual ExactSize &lshr(const Domain &a, const Domain &b);

    virtual ExactSize &ashr(const Domain &a, const Domain &b);

    virtual ExactSize &and_(const Domain &a, const Domain &b);

    virtual ExactSize &or_(const Domain &a, const Domain &b);

    virtual ExactSize &xor_(const Domain &a, const Domain &b);

    virtual ExactSize &icmp(const Domain &a, const Domain &b,
                            llvm::CmpInst::Predicate predicate);

    virtual ExactSize &fcmp(const Domain &a, const Domain &b,
                            llvm::CmpInst::Predicate predicate);

public: // Implementation of Array::Interface.
    virtual std::vector<Domain*> getItem(const Domain &offset) const;

    virtual Domain *getItem(uint64_t offset) const;

    virtual void setItem(const Domain &offset, const Domain &value);

    virtual void setItem(uint64_t offset, const Domain &value);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_EXACT_SIZE_H
