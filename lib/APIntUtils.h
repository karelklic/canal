#ifndef LIBCANAL_APINT_UTILS_H
#define LIBCANAL_APINT_UTILS_H

namespace llvm {
    class APInt;
}

namespace Canal {
namespace APIntUtils {

// Minimum and maximum.
llvm::APInt signedMin(const llvm::APInt &a,
                      const llvm::APInt &b,
                      const llvm::APInt &c);

llvm::APInt signedMax(const llvm::APInt &a,
                      const llvm::APInt &b,
                      const llvm::APInt &c);

llvm::APInt unsignedMin(const llvm::APInt &a,
                        const llvm::APInt &b,
                        const llvm::APInt &c);

llvm::APInt unsignedMax(const llvm::APInt &a,
                        const llvm::APInt &b,
                        const llvm::APInt &c);

// APInt compatibility for LLVM 2.8 and older.
// Operations that return overflow indicators.
llvm::APInt sadd_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow);

llvm::APInt uadd_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow);

llvm::APInt ssub_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow);

llvm::APInt usub_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow);

llvm::APInt sdiv_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow);

llvm::APInt smul_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow);

llvm::APInt umul_ov(const llvm::APInt &a,
                    const llvm::APInt &b,
                    bool &overflow);

} // namespace APIntUtils
} // namespace Canal

#endif // LIBCANAL_APINT_UTILS_H
