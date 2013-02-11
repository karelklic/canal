#ifndef LIBCANAL_DOMAIN_H
#define LIBCANAL_DOMAIN_H

#include "SharedDataPointer.h"
#include <cstddef>
#include <string>

namespace Canal {

/// @brief
///   Base class for all abstract domains.
class Domain : public SharedData
{
public:
    typedef Domain&(Domain::*CastOperation)(const Domain&);

    typedef Domain&(Domain::*BinaryOperation)(const Domain&,
                                              const Domain&);

    typedef Domain&(Domain::*CmpOperation)(const Domain&,
                                           const Domain&,
                                           llvm::CmpInst::Predicate predicate);

    /// Discriminator for LLVM-style RTTI (dyn_cast<> et al.)
    enum DomainKind
    {
        ArrayExactSizeKind,
        ArraySingleItemKind,
        ArrayStringPrefixKind,
        FloatIntervalKind,
        ProductVectorKind,
        IntegerBitfieldKind,
        IntegerIntervalKind,
        IntegerSetKind,
        PointerKind,
        StructureKind
    };

    const Environment &mEnvironment;

    const DomainKind mKind;

    Widening::DataInterface *mWideningData;

    /// Can be NULL for domains which are not representing a value.
    const llvm::Type *mType;

public:
    /// Standard constructor.
    Domain(const Environment &environment,
           DomainKind kind);

    /// Copy constructor.  Careful!  Copy constructor of base class is
    /// not called by automatically generated copy constructor of
    /// an inherited class.
    Domain(const Domain &value);

    /// Virtual destructor.
    virtual ~Domain();

    const Environment &getEnvironment() const
    {
        return mEnvironment;
    }

    DomainKind getKind() const
    {
        return mKind;
    }

    /// Create a copy of this value.
    virtual Domain *clone() const = 0;

    /// Get memory usage (used byte count) of this abstract value.
    virtual size_t memoryUsage() const = 0;

    /// Create a string representation of the abstract value.
    virtual std::string toString() const = 0;

    /// Set value of this domain to represent zeroed memory.  Needed
    /// for constants with zero initializer.  This can only be called
    /// right after creating a domain.
    virtual void setZero(const llvm::Value *place);

public: // Lattice
    /// Implementing this is mandatory.  Values are compared while
    /// computing the fixed point.
    virtual bool operator==(const Domain &value) const = 0;

    /// Inequality is implemented by calling the equality operator.
    virtual bool operator!=(const Domain &value) const
    {
        return !operator==(value);
    }

    virtual bool operator<(const Domain &value) const = 0;

    virtual bool operator<=(const Domain &value) const
    {
        return operator==(value) || operator<(value);
    }

    /// Merge another value into this one.
    virtual Domain &join(const Domain &value) = 0;

    virtual Domain &meet(const Domain &value) = 0;

    /// Is it the lowest value.
    virtual bool isBottom() const;

    /// Set to the lowest value.
    virtual void setBottom();

    /// Is it the highest value.
    virtual bool isTop() const;

    /// Set it to the top value of lattice.
    virtual void setTop();

    /// Get accuracy of the abstract value (0 - 1). In finite-height
    /// lattices, it is determined by the position of the value in the
    /// lattice.
    ///
    /// Accuracy 0 means that the value represents all possible values
    /// (top).  Accuracy 1 means that the value represents the most
    /// precise and exact value (bottom).
    virtual float accuracy() const;

public: // Instructions operating on values.
    virtual Domain &add(const Domain &a, const Domain &b);

    virtual Domain &fadd(const Domain &a, const Domain &b);

    virtual Domain &sub(const Domain &a, const Domain &b);

    virtual Domain &fsub(const Domain &a, const Domain &b);

    virtual Domain &mul(const Domain &a, const Domain &b);

    virtual Domain &fmul(const Domain &a, const Domain &b);

    /// Unsigned division.
    /// Stores the result of unsigned division a / b to this object.
    virtual Domain &udiv(const Domain &a, const Domain &b);

    /// Signed division.
    virtual Domain &sdiv(const Domain &a, const Domain &b);

    /// Floating point division.
    virtual Domain &fdiv(const Domain &a, const Domain &b);

    virtual Domain &urem(const Domain &a, const Domain &b);

    virtual Domain &srem(const Domain &a, const Domain &b);

    virtual Domain &frem(const Domain &a, const Domain &b);

    virtual Domain &shl(const Domain &a, const Domain &b);

    virtual Domain &lshr(const Domain &a, const Domain &b);

    virtual Domain &ashr(const Domain &a, const Domain &b);

    virtual Domain &and_(const Domain &a, const Domain &b);

    virtual Domain &or_(const Domain &a, const Domain &b);

    virtual Domain &xor_(const Domain &a, const Domain &b);

    virtual Domain &icmp(const Domain &a,
                         const Domain &b,
                         llvm::CmpInst::Predicate predicate);

    virtual Domain &fcmp(const Domain &a,
                         const Domain &b,
                         llvm::CmpInst::Predicate predicate);

    virtual Domain &trunc(const Domain &value);

    virtual Domain &zext(const Domain &value);

    virtual Domain &sext(const Domain &value);

    virtual Domain &fptrunc(const Domain &value);

    virtual Domain &fpext(const Domain &value);

    virtual Domain &fptoui(const Domain &value);

    virtual Domain &fptosi(const Domain &value);

    virtual Domain &uitofp(const Domain &value);

    virtual Domain &sitofp(const Domain &value);

    /// Extracts a single element from an array at a specified index.
    /// The element is returned.  Caller is responsible for deleting it.
    virtual Domain *extractelement(const Domain &index) const;

    /// Inserts an element into an array at a specified index.
    virtual Domain &insertelement(const Domain &array,
                                  const Domain &element,
                                  const Domain &index);

    /// Constructs a permutation of elements from two input vectors,
    /// returning a vector with the same element type as the input and
    /// length that is the same as the shuffle mask.
    /// @param a
    /// @param b
    ///   Arrays with the same type.
    /// @param mask
    ///   A shuffle mask whose element type is always 'i32'.
    virtual Domain &shufflevector(const Domain &a,
                                  const Domain &b,
                                  const std::vector<uint32_t> &mask);

    /// Extracts the value of a member field from an aggregate value.
    virtual Domain *extractvalue(const std::vector<unsigned> &indices) const;

    /// Inserts a value into a member field in an aggregate value.
    virtual Domain &insertvalue(const Domain &aggregate,
                                const Domain &element,
                                const std::vector<unsigned> &indices);

    /// Inserts a value into a member field in an aggregate value.
    virtual void insertvalue(const Domain &element,
                             const std::vector<unsigned> &indices);

public: // Widening interface.
    Widening::DataInterface *getWideningData() const
    {
        return mWideningData;
    }

    /// This class takes ownership of the wideningData memory.
    void setWideningData(Widening::DataInterface *wideningData);

public: // Memory layout.
    /// @brief
    ///   Returns the type of the concrete values represented by this
    ///   abstract domain.
    ///
    /// If the abstract domain does not represent a value, the
    /// behavior is undefined.
    virtual const llvm::Type &getValueType() const;

    /// @brief
    ///   Indicates whether all concrete values represented by this
    ///   abstract domain use an exact number of bytes of memory to be
    ///   stored.
    ///
    /// This is true for abstract domains representing numbers,
    /// structures, vectors and fixed-size arrays.  It's false for
    /// variable-sized arrays.
    virtual bool hasValueExactSize() const;

    virtual uint64_t getValueExactSize();

    virtual Domain *getValueAbstractSize();

    virtual Domain *getValueCell(uint64_t offset) const;

    virtual void mergeValueCell(uint64_t offset, const Domain &value);

    /// Checks whether the abstract value represents an abstract value
    /// of the provided type on the provided abstract memory offset.
    /// If the abstract memory offset can be concretized to multiple
    /// concrete offsets, it is checked whether a value is represented
    /// by this abstract value for each of the concrete offsets.
    ///
    /// @param type
    ///   Type of the abstract value.
    /// @param offset
    ///   Offset in bytes.
    /// @note
    ///   The default implementation that is useful for primitive
    ///   types.
    virtual bool containsValue(const llvm::Type &type,
                               const Domain &offset) const;

    /// @param offset
    ///   Offset in bytes.
    /// @note
    ///   The default implementation that is useful for primitive
    ///   types.
    virtual Domain *loadValue(const llvm::Type &type,
                              const Domain &offset) const;

    /// @param offset
    ///   Offset in bytes.
    /// @param isSingleTarget
    ///   Helps to decide whether to set the value or join it to the
    ///   existing value.
    /// @note
    ///   The default implementation that is useful for primitive
    ///   types.
    virtual void storeValue(const Domain &value,
                            const Domain &offset,
                            bool isSingleTarget);

public: // Product of abstract domains.
    /// The default implementation extracts no information from the
    /// abstract value to the message.
    virtual void extract(Product::Message &message) const
    {
    }

    /// The default implementation does not enrich the abstract value
    /// by the contents of the message.
    virtual void refine(const Product::Message &message)
    {
    }

private: // Domains are non-copyable.
    /// Assignment operator declaration.  Prevents accidental
    /// assignments of domains.  Do not implement!
    Domain &operator=(const Domain &value);
};

} // namespace Canal

#endif // LIBCANAL_DOMAIN_H
