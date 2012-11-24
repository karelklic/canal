#ifndef LIBCANAL_FLOAT_UTILS_H
#define LIBCANAL_FLOAT_UTILS_H

#include "Prereq.h"

namespace Canal {
namespace Float {
namespace Utils {

const llvm::fltSemantics &getSemantics(const llvm::Type &type);

const llvm::Type &getType(const llvm::fltSemantics &semantics,
                          llvm::LLVMContext &context);

} // namespace Utils
} // namespace Float
} // namespace Canal

#endif // LIBCANAL_FLOAT_UTILS_H
