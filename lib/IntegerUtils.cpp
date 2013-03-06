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

llvm::APInt
sadd_ov(const llvm::APInt &a,
        const llvm::APInt &b,
        bool &overflow)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    llvm::APInt result = a + b;
    overflow = a.isNonNegative() == b.isNonNegative() &&
        result.isNonNegative() != a.isNonNegative();

    return result;
#else
    return a.sadd_ov(b, overflow);
#endif
}

llvm::APInt
uadd_ov(const llvm::APInt &a,
        const llvm::APInt &b,
        bool &overflow)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    llvm::APInt result = a+b;
    overflow = result.ult(b);
    return result;
#else
    return a.uadd_ov(b, overflow);
#endif
}

llvm::APInt
ssub_ov(const llvm::APInt &a,
        const llvm::APInt &b,
        bool &overflow)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    llvm::APInt result = a - b;
    overflow = a.isNonNegative() != b.isNonNegative() &&
        result.isNonNegative() != a.isNonNegative();

    return result;
#else
    return a.ssub_ov(b, overflow);
#endif
}

llvm::APInt
usub_ov(const llvm::APInt &a,
        const llvm::APInt &b,
        bool &overflow)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    llvm::APInt result = a-b;
    overflow = result.ugt(a);
    return result;
#else
    return a.usub_ov(b, overflow);
#endif
}

llvm::APInt
sdiv_ov(const llvm::APInt &a,
        const llvm::APInt &b,
        bool &overflow)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    // MININT/-1  -->  overflow.
    overflow = a.isMinSignedValue() && b.isAllOnesValue();
    return a.sdiv(b);
#else
    return a.sdiv_ov(b, overflow);
#endif
}

llvm::APInt
smul_ov(const llvm::APInt &a,
        const llvm::APInt &b,
        bool &overflow)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    llvm::APInt result = a * b;
    if (a != 0 && b != 0)
        overflow = result.sdiv(b) != a || result.sdiv(a) != b;
    else
        overflow = false;

    return result;
#else
    return a.smul_ov(b, overflow);
#endif
}

llvm::APInt
umul_ov(const llvm::APInt &a,
        const llvm::APInt &b,
        bool &overflow)
{
#if LLVM_VERSION_MAJOR == 2
    llvm::APInt result = a * b;
    if (a != 0 && b != 0)
        overflow = result.udiv(b) != a || result.udiv(a) != b;
    else
        overflow = false;

    return result;
#else
    return a.umul_ov(b, overflow);
#endif
}

void
clearAllBits(llvm::APInt &num)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    num.clear(num.getBitWidth());
#else
    num.clearAllBits();
#endif
}

void
setBit(llvm::APInt &num, int bit)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    num.set(bit);
#else
    num.setBit(bit);
#endif
}

llvm::APInt
getOneBitSet(unsigned bitWidth, int bit)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    return llvm::APInt::getBitsSet(bitWidth, bit, bit + 1);
#else
    return llvm::APInt::getOneBitSet(bitWidth, bit);
#endif
}

llvm::APInt
trunc(const llvm::APInt &num, unsigned bitWidth)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    llvm::APInt copy(num);
    return copy.trunc(bitWidth);
#else
    return num.trunc(bitWidth);
#endif
}

llvm::APInt
zext(const llvm::APInt &num, unsigned bitWidth)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    llvm::APInt copy(num);
    return copy.zext(bitWidth);
#else
    return num.zext(bitWidth);
#endif
}

llvm::APInt
sext(const llvm::APInt &num, unsigned bitWidth)
{
#if LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9
    llvm::APInt copy(num);
    return copy.sext(bitWidth);
#else
    return num.sext(bitWidth);
#endif
}

} // namespace Utils
} // namespace Integer
} // namespace Canal
