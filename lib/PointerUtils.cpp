#include "PointerUtils.h"
#include "Pointer.h"
#include "Utils.h"

namespace Canal {
namespace Pointer {
namespace Utils {

void
addTarget(Canal::Domain &pointer,
          Target::Type type,
          const llvm::Value *place,
          const llvm::Value *target,
          const std::vector<Domain *> &offsets,
          Canal::Domain *numericOffset)
{
    checkedCast<Pointer>(pointer).addTarget(type,
                                            place,
                                            target,
                                            offsets,
                                            numericOffset);
}

} // namespace Utils
} // namespace Pointer
} // namespace Canal
