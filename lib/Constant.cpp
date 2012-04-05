#include "Constant.h"
#include <llvm/Constant.h>

namespace Canal {

Constant::Constant(const llvm::Constant *constant) : mConstant(constant)
{
}

Constant* Constant::clone() const
{
    return new Constant(*this);
}

bool Constant::operator==(const Value &value) const
{
    // Check if rhs has the same type.
    const Constant *constant = dynamic_cast<const Constant*>(&value);
    if (!constant)
        return false;

    return mConstant == constant.mConstant;
}

} // namespace Canal
