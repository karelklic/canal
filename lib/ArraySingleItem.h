#ifndef LIBCANAL_ARRAY_SINGLE_ITEM_H
#define LIBCANAL_ARRAY_SINGLE_ITEM_H

#include "Value.h"
#include "ArrayInterface.h"

namespace Canal {
namespace Array {

// The most trivial array type.  It treats all array members as a
// single value.  This means all the operations on the array are
// merged and used to move the single value up in its lattice.
//
// This array type is very imprecise.
class SingleItem : public Value, public Interface
{
public:
    Value *mValue;

    // Number of elements in the array.
    // It is either a Constant or Integer::Container.
    Value *mSize;

public:
    SingleItem();
    SingleItem(const SingleItem &singleItem);
    virtual ~SingleItem();

public: // Implementation of Value.
    // Implementation of Value::clone().
    // Covariant return type.
    virtual SingleItem *clone() const;
    // Implementation of Value::cloneCleaned().
    // Covariant return type.
    virtual SingleItem *cloneCleaned() const;
    // Implementation of Value::operator==().
    virtual bool operator==(const Value &value) const;
    // Implementation of Value::merge().
    virtual void merge(const Value &value);
    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const;
    // Implementation of Value::toString().
    virtual std::string toString() const;

    // Implementation of Value::add().
    virtual void add(const Value &a, const Value &b);
    // Implementation of Value::fadd().
    virtual void fadd(const Value &a, const Value &b);
    // Implementation of Value::sub().
    virtual void sub(const Value &a, const Value &b);
    // Implementation of Value::fsub().
    virtual void fsub(const Value &a, const Value &b);
    // Implementation of Value::mul().
    virtual void mul(const Value &a, const Value &b);
    // Implementation of Value::fmul().
    virtual void fmul(const Value &a, const Value &b);
    // Implementation of Value::udiv().
    virtual void udiv(const Value &a, const Value &b);
    // Implementation of Value::sdiv().
    virtual void sdiv(const Value &a, const Value &b);
    // Implementation of Value::fdiv().
    virtual void fdiv(const Value &a, const Value &b);
    // Implementation of Value::urem().
    virtual void urem(const Value &a, const Value &b);
    // Implementation of Value::srem().
    virtual void srem(const Value &a, const Value &b);
    // Implementation of Value::frem().
    virtual void frem(const Value &a, const Value &b);
    // Implementation of Value::shl().
    virtual void shl(const Value &a, const Value &b);
    // Implementation of Value::lshr().
    virtual void lshr(const Value &a, const Value &b);
    // Implementation of Value::ashr().
    virtual void ashr(const Value &a, const Value &b);
    // Implementation of Value::and_().
    virtual void and_(const Value &a, const Value &b);
    // Implementation of Value::or_().
    virtual void or_(const Value &a, const Value &b);
    // Implementation of Value::xor_().
    virtual void xor_(const Value &a, const Value &b);
    // Implementation of Value::icmp().
    virtual void icmp(const Value &a, const Value &b,
                      llvm::CmpInst::Predicate predicate);
    // Implementation of Value::fcmp().
    virtual void fcmp(const Value &a, const Value &b,
                      llvm::CmpInst::Predicate predicate);

public: // Implementation of Array::Interface.
    // Implementation of Array::Interface::getItem().
    virtual std::vector<Value*> getItem(const Value &offset) const;
    // Implementation of Array::Interface::getItem().
    virtual Value *getItem(uint64_t offset) const;
    // Implementation of Array::Interface::setItem().
    virtual void setItem(const Value &offset, const Value &value);
    // Implementation of Array::Interface::setItem().
    virtual void setItem(uint64_t offset, const Value &value);
};

} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_SINGLE_ITEM_H
