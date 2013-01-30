#ifndef LIBCANAL_FLOAT_UTILS_H
#define LIBCANAL_FLOAT_UTILS_H

#include "Prereq.h"

namespace Canal {
namespace Float {
namespace Utils {

const llvm::fltSemantics &getSemantics(const llvm::Type &type);

const llvm::Type &getType(const llvm::fltSemantics &semantics,
                          llvm::LLVMContext &context);

llvm::APInt toInteger(const llvm::APFloat &num,
                      unsigned bitWidth,
                      bool isSigned,
                      llvm::APFloat::opStatus &status);

} // namespace Utils
} // namespace Float
} // namespace Canal

#endif // LIBCANAL_FLOAT_UTILS_H
