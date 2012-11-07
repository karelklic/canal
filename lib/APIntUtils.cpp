#include "APIntUtils.h"
#include <llvm/Constants.h>

namespace Canal {
namespace APIntUtils {

llvm::APInt
sadd_ov(const llvm::APInt &a,
        const llvm::APInt &b,
        bool &overflow)
{
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
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
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
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
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
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
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
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
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
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
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
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
#if LLVM_MAJOR == 2
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
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
    num.clear(num.getBitWidth());
#else
    num.clearAllBits();
#endif
}

void
setBit(llvm::APInt &num, int bit)
{
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
    num.set(bit);
#else
    num.setBit(bit);
#endif
}

llvm::APInt
getOneBitSet(unsigned bitWidth, int bit)
{
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
    return llvm::APInt::getBitsSet(bitWidth, bit, bit + 1);
#else
    return llvm::APInt::getOneBitSet(bitWidth, bit);
#endif
}

llvm::APInt
trunc(const llvm::APInt &num, unsigned bitWidth)
{
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
    llvm::APInt copy(num);
    return copy.trunc(bitWidth);
#else
    return num.trunc(bitWidth);
#endif
}

llvm::APInt
zext(const llvm::APInt &num, unsigned bitWidth)
{
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
    llvm::APInt copy(num);
    return copy.zext(bitWidth);
#else
    return num.zext(bitWidth);
#endif
}

llvm::APInt
sext(const llvm::APInt &num, unsigned bitWidth)
{
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
    llvm::APInt copy(num);
    return copy.sext(bitWidth);
#else
    return num.sext(bitWidth);
#endif
}

} // namespace APIntUtils
} // namespace Canal
