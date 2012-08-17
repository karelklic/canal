#ifndef LIBCANAL_APINT_UTILS_H
#define LIBCANAL_APINT_UTILS_H

#include <set>
#include <llvm/ADT/APInt.h>

namespace Canal {
namespace APIntUtils {

struct UCompare
{
    bool operator()(const llvm::APInt &a,
                    const llvm::APInt &b) const
    {
        return a.ult(b);
    }
};

struct SCompare
{
    bool operator()(const llvm::APInt &a,
                    const llvm::APInt &b) const
    {
        return a.slt(b);
    }
};

typedef std::set<llvm::APInt, UCompare> USet;

typedef llvm::APInt(llvm::APInt::*Operation)(const llvm::APInt&) const;
typedef llvm::APInt(llvm::APInt::*OperationWithOverflow)(const llvm::APInt&,
                                                         bool&) const;


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

void clearAllBits(llvm::APInt &num);

void setBit(llvm::APInt &num, int bit);

llvm::APInt getOneBitSet(unsigned bitWidth, int bit);

} // namespace APIntUtils
} // namespace Canal

#endif // LIBCANAL_APINT_UTILS_H
