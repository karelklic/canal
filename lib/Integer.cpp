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

} // namespace Integer
} // namespace Canal
