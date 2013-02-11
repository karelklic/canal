#ifndef LIBCANAL_INTEGER_UTILS_H
#define LIBCANAL_INTEGER_UTILS_H

#include "Prereq.h"

namespace Canal {
namespace Integer {
namespace Utils {

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

/// @brief
///   Find out whether the abstract number represents only a single
///   concrete value.
///
/// It check that at least one of the integer abstract value
/// representations represents a single concrete value.
///
/// @param value
///   Abstract value that represents number.
/// @returns
///   If true is returned, the single concrete value can be obtained
///   by calling the getConstant function.
bool isConstant(const Domain &value);

/// @brief
///   Returns the single concrete value represented by an abstract
///   number.
///
/// If the abstract number does not represent a single concrete value,
/// the behaviour is undefined.  Call isConstant function to check
/// whether getConstant can be safely called.
llvm::APInt getConstant(const Domain &value);

} // namespace Utils
} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_UTILS_H
