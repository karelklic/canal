#ifndef LIBCANAL_INTEGER_UTILS_H
#define LIBCANAL_INTEGER_UTILS_H

#include "Prereq.h"
#include <set>

namespace Canal {
namespace Integer {
namespace Utils {

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

unsigned getBitWidth(const Domain &value);

Bitfield &getBitfield(Domain &value);

const Bitfield &getBitfield(const Domain &value);

Set &getSet(Domain &value);

const Set &getSet(const Domain &value);

Interval &getInterval(Domain &value);

const Interval &getInterval(const Domain &value);

/// Lowest signed number represented by this container.  Uses the
/// abstract domain (enum, interval, bits) with highest precision.
/// @param value
///   Abstract value that represents number.
/// @param result
///   Filled by the minimum value if it is known.  Otherwise, the
///   value is undefined.
/// @return
///   True if the result is known and the parameter was set to
///   correct value.
bool signedMin(const Domain &value, llvm::APInt &result);

/// Highest signed number represented by this container.  Uses the
/// abstract domain (enum, interval, bits) with highest precision.
/// @param value
///   Abstract value that represents number.
/// @param result
///   Filled by the maximum value if it is known.  Otherwise, the
///   value is undefined.
/// @return
///   True if the result is known and the parameter was set to
///   correct value.
bool signedMax(const Domain &value, llvm::APInt &result);

/// Lowest unsigned number represented by this container.  Uses the
/// abstract domain (enum, interval, bits) with highest precision.
/// @param value
///   Abstract value that represents number.
/// @param result
///   Filled by the minimum value if it is known.  Otherwise, the
///   value is undefined.
/// @return
///   True if the result is known and the parameter was set to
///   correct value.

bool unsignedMin(const Domain &value, llvm::APInt &result);
/// Highest unsigned number represented by this container.  Uses
/// the abstract domain (enum, interval, bits) with highest precision.
/// @param value
///   Abstract value that represents number.
/// @param result
///   Filled by the maximum value if it is known.  Otherwise, the
///   value is undefined.
/// @return
///   True if the result is known and the parameter was set to
///   correct value.

bool unsignedMax(const Domain &value, llvm::APInt &result);

/// Find out whether all representations contain only single value
/// @param value
///   Abstract value that represents number.
bool isConstant(const Domain &value);

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

void
clearAllBits(llvm::APInt &num);

void
setBit(llvm::APInt &num, int bit);

llvm::APInt
getOneBitSet(unsigned bitWidth, int bit);

llvm::APInt
trunc(const llvm::APInt &num, unsigned bitWidth);

llvm::APInt
zext(const llvm::APInt &num, unsigned bitWidth);

llvm::APInt
sext(const llvm::APInt &num, unsigned bitWidth);

} // namespace Utils
} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_UTILS_H
