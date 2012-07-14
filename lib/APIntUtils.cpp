#include "APIntUtils.h"
#include <llvm/Constants.h>

namespace Canal {
namespace APIntUtils {

llvm::APInt sadd_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow)
{
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
    llvm::APInt result = a+b;
    overflow = a.isNonNegative() == b.isNonNegative() &&
        result.isNonNegative() != a.isNonNegative();
    return result;
#else
    return a.sadd_ov(b, overflow);
#endif
}

llvm::APInt uadd_ov(const llvm::APInt &a,
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

llvm::APInt ssub_ov(const llvm::APInt &a,
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

llvm::APInt usub_ov(const llvm::APInt &a,
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

llvm::APInt sdiv_ov(const llvm::APInt &a,
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

llvm::APInt smul_ov(const llvm::APInt &a,
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

llvm::APInt umul_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow)
{
#if LLVM_MAJOR == 2 && LLVM_MINOR < 9
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


} // namespace APIntUtils
} // namespace Canal
