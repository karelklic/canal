#include "IntegerUtils.h"

#include "IntegerBitfield.h"
#include "ProductVector.h"
#include "IntegerSet.h"
#include "IntegerInterval.h"
#include "Utils.h"

namespace Canal {
namespace Integer {
namespace Utils {

unsigned
getBitWidth(const Domain &value)
{
    const Set &integer = getSet(value);
    return integer.getBitWidth();
}

Bitfield &
getBitfield(Domain &value)
{
    Product::Vector &container = checkedCast<Product::Vector>(value);
    return checkedCast<Bitfield>(*container.mValues[0]);
}

const Bitfield &
getBitfield(const Domain &value)
{
    const Product::Vector &container = checkedCast<Product::Vector>(value);
    return checkedCast<Bitfield>(*container.mValues[0]);
}

Set &
getSet(Domain &value)
{
    Product::Vector &container = checkedCast<Product::Vector>(value);
    return checkedCast<Set>(*container.mValues[1]);
}

const Set &
getSet(const Domain &value)
{
    const Product::Vector &container = checkedCast<Product::Vector>(value);
    return checkedCast<Set>(*container.mValues[1]);
}

Interval &
getInterval(Domain &value)
{
    Product::Vector &container = checkedCast<Product::Vector>(value);
    return checkedCast<Interval>(*container.mValues[2]);
}

const Interval &
getInterval(const Domain &value)
{
    const Product::Vector &container = checkedCast<Product::Vector>(value);
    return checkedCast<Interval>(*container.mValues[2]);
}

bool
signedMin(const Domain &value, llvm::APInt &result)
{
    if (!getSet(value).signedMin(result))
        return false;

    llvm::APInt temp;
    if (!getInterval(value).signedMin(temp))
        return false;

    // If the minimum returned by the interval is higher, it means it
    // is also more precise.
    if (result.slt(temp))
        result = temp;

    if (!getBitfield(value).signedMin(temp))
        return false;

    if (result.slt(temp))
        result = temp;

    return true;
}

bool
signedMax(const Domain &value, llvm::APInt &result)
{
    if (!getSet(value).signedMax(result))
        return false;

    llvm::APInt temp;
    if (!getInterval(value).signedMax(temp))
        return false;

    if (result.sgt(temp))
        result = temp;

    if (!getBitfield(value).signedMax(temp))
        return false;

    if (result.sgt(temp))
        result = temp;

    return true;
}

bool
unsignedMin(const Domain &value, llvm::APInt &result)
{
    if (!getSet(value).unsignedMin(result))
        return false;

    llvm::APInt temp;
    if (!getInterval(value).unsignedMin(temp))
        return false;

    if (result.ult(temp))
        result = temp;

    if (!getBitfield(value).unsignedMin(temp))
        return false;

    if (result.ult(temp))
        result = temp;

    return true;
}

bool
unsignedMax(const Domain &value, llvm::APInt &result)
{
    if (!getSet(value).unsignedMax(result))
        return false;

    llvm::APInt temp;
    if (!getInterval(value).unsignedMax(temp))
        return false;

    if (result.ugt(temp))
        result = temp;

    if (!getBitfield(value).unsignedMax(temp))
        return false;

    if (result.ugt(temp))
        result = temp;

    return true;
}

bool
isConstant(const Domain &value)
{
    return getBitfield(value).isConstant()
        && getSet(value).isConstant()
        && getInterval(value).isConstant();
}

} // namespace Utils
} // namespace Integer
} // namespace Canal
