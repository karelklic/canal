#include "Integer.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Container::Container(unsigned numBits) : mBits(numBits)
{
}

Container::Container(const llvm::APInt &number) : mBits(number)
{
}

Container::Container(const Container &container) : mBits(container.mBits)
{
}

Container::~Container()
{
}

Container *
Container::clone() const
{
    return new Container(*this);
}

bool
Container::operator==(const Value &value) const
{
    const Container *container = dynamic_cast<const Container*>(&value);
    if (!container)
        return false;
    if (mBits != container->mBits)
        return false;
    return true;
}

void
Container::merge(const Value &value)
{
    if (const Constant *constant = dynamic_cast<const Constant*>(&value))
    {
        mBits.merge(*constant);
        return;
    }

    const Container &container = dynamic_cast<const Container&>(value);
    mBits.merge(container.mBits);
}

size_t
Container::memoryUsage() const
{
    size_t size = mBits.memoryUsage();
    return size;
}

std::string
Container::toString() const
{
    std::stringstream ss;
    ss << "Integer::Container: {" << std::endl;
    ss << "    bits: " << indentExceptFirstLine(mBits.toString(), 10) << std::endl;
    ss << "}";
    return ss.str();
}

void
Container::add(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::sub(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::mul(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::udiv(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::sdiv(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::urem(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::srem(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::shl(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::lshr(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::ashr(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.setTop();
}

void
Container::and_(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.and_(aa.mBits, bb.mBits);
}

void
Container::or_(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.or_(aa.mBits, bb.mBits);
}

void
Container::xor_(const Value &a, const Value &b)
{
    const Container &aa = dynamic_cast<const Container&>(a);
    const Container &bb = dynamic_cast<const Container&>(b);
    mBits.xor_(aa.mBits, bb.mBits);
}

} // namespace Integer
} // namespace Canal
