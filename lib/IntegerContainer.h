#ifndef LIBCANAL_INTEGER_CONTAINER_H
#define LIBCANAL_INTEGER_CONTAINER_H

#include "Domain.h"

namespace Canal {
namespace Integer {

class Bitfield;
class Enumeration;
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

    unsigned getBitWidth() const;

    Bitfield &getBitfield();
    const Bitfield &getBitfield() const;

    Enumeration &getEnumeration();
    const Enumeration &getEnumeration() const;

    Interval &getInterval();
    const Interval &getInterval() const;

    /// Lowest signed number represented by this container.  Uses the
    /// abstract domain (enum, interval, bits) with highest precision.
    /// @param result
    ///   Filled by the minimum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool signedMin(llvm::APInt &result) const;

    /// Highest signed number represented by this container.  Uses the
    /// abstract domain (enum, interval, bits) with highest precision.
    /// @param result
    ///   Filled by the maximum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool signedMax(llvm::APInt &result) const;

    /// Lowest unsigned number represented by this container.  Uses the
    /// abstract domain (enum, interval, bits) with highest precision.
    /// @param result
    ///   Filled by the minimum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool unsignedMin(llvm::APInt &result) const;

    /// Highest unsigned number represented by this container.  Uses
    /// the abstract domain (enum, interval, bits) with highest precision.
    /// @param result
    ///   Filled by the maximum value if it is known.  Otherwise, the
    ///   value is undefined.
    /// @return
    ///   True if the result is known and the parameter was set to
    ///   correct value.
    bool unsignedMax(llvm::APInt &result) const;

    /// Find out whether all representations contain only single value
    bool isSingleValue() const;

public: // Implementation of Domain.
    /// Covariant return type.
    virtual Container *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

    virtual bool operator>(const Domain &value) const;

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

    virtual bool isValue() const;

    virtual const llvm::Type &getValueType() const;

    virtual bool hasValueExactSize() const;

    virtual Domain *getValueAbstractSize() const;

    virtual Domain *getValueCell(uint64_t offset) const;

    virtual void mergeValueCell(uint64_t offset, const Domain &value);
};

} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_H
