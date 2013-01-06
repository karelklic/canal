#include "PointerUtils.h"
#include "Pointer.h"
#include "Utils.h"

namespace Canal {
namespace Pointer {

void
Utils::addTarget(Canal::Domain &pointer,
                 Target::Type type,
                 const llvm::Value *place,
                 const llvm::Value *target,
                 const std::vector<Domain *> &offsets,
                 Canal::Domain *numericOffset)
{
    llvm::cast<Pointer>(pointer).addTarget(type,
                                           place,
                                           target,
                                           offsets,
                                           numericOffset);
}

} // namespace Pointer
} // namespace Canal
