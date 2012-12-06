#ifndef LIBCANAL_INTEGER_UTILS_H
#define LIBCANAL_INTEGER_UTILS_H

#include "Prereq.h"

namespace Canal {
class Domain;

namespace Integer {

class Bitfield;
class Set;
class Interval;

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

/// Find out whether all representations contain only single value
/// @param value
///   Abstract value that represents number.
bool isConstant(const Domain &value);

} // namespace Utils
} // namespace Integer
} // namespace Canal

#endif // LIBCANAL_INTEGER_UTILS_H
