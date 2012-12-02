#include "Memory.h"
#include "Utils.h"

namespace Canal {

Memory::Memory(Domain *value)
    : Domain(value->getEnvironment()), mValue(value)
{
}

Memory::Memory(const Memory &value)
    : Domain(value.getEnvironment()), mValue(value.mValue)
{
    mValue = mValue->clone();
}

Memory::~Memory()
{
    delete mValue;
}

Domain *
Memory::load(const llvm::Type &type,
             const std::vector<Domain*> &offsets) const
{
}

Domain *
Memory::load(const llvm::Type &type,
             const Domain *numericOffset) const
{
}

void
Memory::store(const Domain &value,
              const std::vector<Domain*> &offsets,
              bool isSingleTarget)
{
}

void
Memory::store(const Domain &value,
              const Domain *numericOffset,
              bool isSingleTarget)
{
}

Memory *
Memory::clone() const
{
    return new Memory(*this);
}

size_t
Memory::memoryUsage() const
{
    return sizeof(Memory) + mValue->memoryUsage();
}

std::string
Memory::toString() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Memory::setZero(const llvm::Value *place)
{
    mValue->setZero(place);
}

bool
Memory::operator==(const Domain& value) const
{
    if (this == &value)
        return true;

    const Memory *other = dynCast<const Memory*>(&value);
    if (!other)
        return false;

    return *mValue == *other->mValue;
}

bool
Memory::operator<(const Domain &value) const
{
    if (this == &value)
        return false;

    const Memory *other = dynCast<const Memory*>(&value);
    if (!other)
        return false;

    return *mValue < *other->mValue;
}

Domain &
Memory::join(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Memory::meet(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Memory::isBottom() const
{
    return mValue->isBottom();
}

void
Memory::setBottom()
{
    mValue->setBottom();
}

bool
Memory::isTop() const
{
    return mValue->isTop();
}

void
Memory::setTop()
{
    mValue->setTop();
}

float
Memory::accuracy() const
{
    return mValue->accuracy();
}

Domain &
Memory::add(const Domain &a, const Domain &b)
{
    mValue->add(a, b);
    return *this;
}

Domain &
Memory::fadd(const Domain &a, const Domain &b)
{
    mValue->fadd(a, b);
    return *this;
}

Domain &
Memory::sub(const Domain &a, const Domain &b)
{
    mValue->sub(a, b);
    return *this;
}

Domain &
Memory::fsub(const Domain &a, const Domain &b)
{
    mValue->fsub(a, b);
    return *this;
}

Domain &
Memory::mul(const Domain &a, const Domain &b)
{
    mValue->mul(a, b);
    return *this;
}

Domain &
Memory::fmul(const Domain &a, const Domain &b)
{
    mValue->fmul(a, b);
    return *this;
}

Domain &
Memory::udiv(const Domain &a, const Domain &b)
{
    mValue->udiv(a, b);
    return *this;
}

Domain &
Memory::sdiv(const Domain &a, const Domain &b)
{
    mValue->sdiv(a, b);
    return *this;
}

Domain &
Memory::fdiv(const Domain &a, const Domain &b)
{
    mValue->fdiv(a, b);
    return *this;
}

Domain &
Memory::urem(const Domain &a, const Domain &b)
{
    mValue->urem(a, b);
    return *this;
}

Domain &
Memory::srem(const Domain &a, const Domain &b)
{
    mValue->srem(a, b);
    return *this;
}

Domain &
Memory::frem(const Domain &a, const Domain &b)
{
    mValue->frem(a, b);
    return *this;
}

Domain &
Memory::shl(const Domain &a, const Domain &b)
{
    mValue->shl(a, b);
    return *this;
}

Domain &
Memory::lshr(const Domain &a, const Domain &b)
{
    mValue->lshr(a, b);
    return *this;
}

Domain &
Memory::ashr(const Domain &a, const Domain &b)
{
    mValue->ashr(a, b);
    return *this;
}

Domain &
Memory::and_(const Domain &a, const Domain &b)
{
    mValue->and_(a, b);
    return *this;
}

Domain &
Memory::or_(const Domain &a, const Domain &b)
{
    mValue->or_(a, b);
    return *this;
}

Domain &
Memory::xor_(const Domain &a, const Domain &b)
{
    mValue->xor_(a, b);
    return *this;
}

Domain &
Memory::icmp(const Domain &a, const Domain &b,
             llvm::CmpInst::Predicate predicate)
{
    mValue->icmp(a, b, predicate);
    return *this;
}

Domain &
Memory::fcmp(const Domain &a, const Domain &b,
             llvm::CmpInst::Predicate predicate)
{
    mValue->fcmp(a, b, predicate);
    return *this;
}

Domain &
Memory::trunc(const Domain &value)
{
    mValue->trunc(value);
    return *this;
}

Domain &
Memory::zext(const Domain &value)
{
    mValue->zext(value);
    return *this;
}

Domain &
Memory::sext(const Domain &value)
{
    mValue->sext(value);
    return *this;
}

Domain &
Memory::fptrunc(const Domain &value)
{
    mValue->fptrunc(value);
    return *this;
}

Domain &
Memory::fpext(const Domain &value)
{
    mValue->fpext(value);
    return *this;
}

Domain &
Memory::fptoui(const Domain &value)
{
    mValue->fptoui(value);
    return *this;
}

Domain &
Memory::fptosi(const Domain &value)
{
    mValue->fptosi(value);
    return *this;
}

Domain &
Memory::uitofp(const Domain &value)
{
    mValue->uitofp(value);
    return *this;
}

Domain &
Memory::sitofp(const Domain &value)
{
    mValue->sitofp(value);
    return *this;
}

} // namespace Canal
