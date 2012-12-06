#ifndef LIBCANAL_ARRAY_EXACT_SIZE_H
#define LIBCANAL_ARRAY_EXACT_SIZE_H

#include "Domain.h"

namespace Canal {
namespace Array {

/// Array with exact size and limited length.  It keeps all array
/// members separately, not losing precision at all.
class ExactSize : public Domain
{
public:
    /// Empty mValues indicate that the ExactSize object is at the top
    /// value because the size is not exact.
    std::vector<Domain*> mValues;

    /// The type of the array.
    const llvm::SequentialType &mType;

public:
    /// Standard constructor.
    ///
    /// If the type is suitable for exact size array, bottom domains
    /// of element type are created in mValues.
    ExactSize(const Environment &environment,
              const llvm::SequentialType &type);

    /// @param values
    ///   This class takes ownership of the values.
    ExactSize(const Environment &environment,
              const llvm::SequentialType &type,
              const std::vector<Domain*> &values);

    /// Copy constructor.
    ExactSize(const ExactSize &value);

    // Standard destructor.
    virtual ~ExactSize();

public: // Implementation of Domain.
    /// Covariant return type.
    virtual ExactSize *clone() const;

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual bool operator==(const Domain &value) const;

    virtual bool operator<(const Domain &value) const;

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

    virtual ExactSize &icmp(const Domain &a,
                            const Domain &b,
                            llvm::CmpInst::Predicate predicate);

    virtual ExactSize &fcmp(const Domain &a,
                            const Domain &b,
                            llvm::CmpInst::Predicate predicate);

    virtual ExactSize &extractelement(const Domain &array,
                                      const Domain &index);

    virtual ExactSize &insertelement(const Domain &array,
                                     const Domain &element,
                                     const Domain &index);

    virtual ExactSize &shufflevector(const Domain &v1,
                                     const Domain &v2,
                                     const std::vector<uint32_t> &mask);

    virtual ExactSize &extractvalue(const Domain &aggregate,
                                    const std::vector<unsigned> &indices);

    virtual ExactSize &insertvalue(const Domain &aggregate,
                                   const Domain &element,
                                   const std::vector<unsigned> &indices);

    virtual bool isValue() const
    {
        return true;
    }

    /// Covariant return type.
    virtual const llvm::SequentialType &getValueType() const;

    virtual bool hasValueExactSize() const
    {
        return !mValues.empty();
    }

    virtual Domain *getValueCell(uint64_t offset) const;

    virtual void mergeValueCell(uint64_t offset, const Domain &value);

private:
    ExactSize &binaryOperation(const Domain &a,
                               const Domain &b,
                               Domain::BinaryOperation operation);

    ExactSize &cmpOperation(const Domain &a,
                            const Domain &b,
                            llvm::CmpInst::Predicate predicate,
                            Domain::CmpOperation operation);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_EXACT_SIZE_H
