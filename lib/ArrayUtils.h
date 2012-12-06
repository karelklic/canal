#ifndef LIBCANAL_ARRAY_UTILS_H
#define LIBCANAL_ARRAY_UTILS_H

#include "Prereq.h"
#include <vector>

namespace Canal {

class Domain;

namespace Array {
namespace Utils {

Domain *extractElement(const Domain &array,
                       const Domain &index);

Domain *extractValue(const Domain &aggregate,
                     const std::vector<unsigned> &indices);

} // namespace Utils
} // namespace Array
} // namespace Canal

#endif // LIBCANAL_ARRAY_UTILS_H
