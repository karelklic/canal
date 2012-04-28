#include "Integer.h"
#include "Utils.h"
#include <sstream>
#include <iostream>

namespace Canal {
namespace Integer {

Container::Container(unsigned numBits) : mBits(new Bits(numBits))
{
}

Container::Container(const llvm::APInt &number) : mBits(new Bits(number))
{
}

Container::Container(const Container &container)
{
    mBits = container.mBits;
    if (mBits)
        mBits = mBits->clone();
}

Container::~Container()
{
    delete mBits;
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
    if ((mBits && !container->mBits) || (!mBits && container->mBits))
        return false;
    if (mBits && *mBits != *container->mBits)
        return false;
    return true;
}

void
Container::merge(const Value &value)
{
    const Container &container = dynamic_cast<const Container&>(value);
    if (mBits)
    {
        if (container.mBits)
            mBits->merge(*container.mBits);
        else
        {
            delete mBits;
            mBits = NULL;
        }
    }
    else
        CANAL_DIE();
}

size_t
Container::memoryUsage() const
{
    size_t size = sizeof(Container);
    if (mBits)
        size += mBits->memoryUsage();
    return size;
}

std::string
Container::toString() const
{
    std::stringstream ss;
    ss << "Integer::Container: {" << std::endl;
    ss << "    bits: " << indentExceptFirstLine(mBits->toString(), 10) << std::endl;
    ss << "}";
    return ss.str();
}

} // namespace Integer
} // namespace Canal
