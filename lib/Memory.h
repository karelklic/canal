#ifndef LIBCANAL_MEMORY_H
#define LIBCANAL_MEMORY_H

#include "Domain.h"

namespace Canal {

// TODO: Add MemoryOverlay class with value of different type and
// stored on an offset.
class Memory : public Domain
{
    Domain *mValue;

public:
    Memory(Domain *value);

    Memory(const Memory &value);

    virtual ~Memory();

    Domain *load(const llvm::Type &type,
                 const std::vector<Domain*> &offsets) const;

    Domain *load(const llvm::Type &type,
                 const Domain *numericOffset) const;

    void store(const Domain &value,
               const std::vector<Domain*> &offsets,
               bool isSingleTarget);

    void store(const Domain &value,
               const Domain *numericOffset,
               bool isSingleTarget);

public: // Implementation of Domain.
    // Covariant return type.
    virtual Memory *clone() const;

    virtual bool operator==(const Domain& value) const;

    virtual void merge(const Domain &value);

    virtual size_t memoryUsage() const;

    virtual std::string toString() const;

    virtual void setZero(const llvm::Value *place);

    virtual Domain &add(const Domain &a, const Domain &b);

    virtual Domain &fadd(const Domain &a, const Domain &b);

    virtual Domain &sub(const Domain &a, const Domain &b);

    virtual Domain &fsub(const Domain &a, const Domain &b);

    virtual Domain &mul(const Domain &a, const Domain &b);

    virtual Domain &fmul(const Domain &a, const Domain &b);

    virtual Domain &udiv(const Domain &a, const Domain &b);

    virtual Domain &sdiv(const Domain &a, const Domain &b);

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

    virtual Domain &icmp(const Domain &a, const Domain &b,
                         llvm::CmpInst::Predicate predicate);

    virtual Domain &fcmp(const Domain &a, const Domain &b,
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

    virtual float accuracy() const;

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();
};

} // namespace Canal

#endif // LIBCANAL_MEMORY_H
