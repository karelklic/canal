
class MemoryOverlay
{
public:
    uint64_t mOffset;
    Domain *mValue;
}

class Memory : public Domain
{
    Domain *mValue;
    std::vector<MemoryOverlay> mOverlays;

public:
    Memory();

    virtual ~Memory();

    Domain *load(const llvm::Type &type,
                 const std::vector<Domain*> &offsets,
                 const Domain *numericOffset) const;

    void store(const Domain &value,
               const std::vector<Domain*> &offsets,
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

    virtual void add(const Domain &a, const Domain &b);

    virtual void fadd(const Domain &a, const Domain &b);

    virtual void sub(const Domain &a, const Domain &b);

    virtual void fsub(const Domain &a, const Domain &b);

    virtual void mul(const Domain &a, const Domain &b);

    virtual void fmul(const Domain &a, const Domain &b);

    virtual void udiv(const Domain &a, const Domain &b);

    virtual void sdiv(const Domain &a, const Domain &b);

    virtual void fdiv(const Domain &a, const Domain &b);

    virtual void urem(const Domain &a, const Domain &b);

    virtual void srem(const Domain &a, const Domain &b);

    virtual void frem(const Domain &a, const Domain &b);

    virtual void shl(const Domain &a, const Domain &b);

    virtual void lshr(const Domain &a, const Domain &b);

    virtual void ashr(const Domain &a, const Domain &b);

    virtual void and_(const Domain &a, const Domain &b);

    virtual void or_(const Domain &a, const Domain &b);

    virtual void xor_(const Domain &a, const Domain &b);

    virtual void icmp(const Domain &a, const Domain &b,
                      llvm::CmpInst::Predicate predicate);

    virtual void fcmp(const Domain &a, const Domain &b,
                      llvm::CmpInst::Predicate predicate);

    virtual void trunc(const Domain &value);

    virtual void zext(const Domain &value);

    virtual void sext(const Domain &value);

    virtual void fptrunc(const Domain &value);

    virtual void fpext(const Domain &value);

    virtual void fptoui(const Domain &value);

    virtual void fptosi(const Domain &value);

    virtual void uitofp(const Domain &value);

    virtual void sitofp(const Domain &value);

    virtual float accuracy() const;

    virtual bool isBottom() const;

    virtual void setBottom();

    virtual bool isTop() const;

    virtual void setTop();

    virtual bool isValue() const;

    virtual bool hasValueExactSize() const;

    virtual uint64_t getValueExactSize();

    virtual Domain *getValueAbstractSize() const;

    virtual Domain *getValueCell(uint64_t offset) const;

    virtual void setValueCell(uint64_t offset, const Domain &value);
};
