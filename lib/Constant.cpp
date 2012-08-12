#include "Constant.h"
#include "IntegerContainer.h"
#include "Utils.h"
#include <sstream>
#include <llvm/Constants.h>
#include <llvm/Support/raw_ostream.h>

namespace Canal {

Constant::Constant(const llvm::Constant *constant) : mConstant(constant)
{
}

bool
Constant::isAPInt() const
{
    return llvm::isa<llvm::ConstantInt>(mConstant);
}

const llvm::APInt &
Constant::getAPInt() const
{
    return llvmCast<llvm::ConstantInt>(mConstant)->getValue();
}

bool
Constant::isGetElementPtr() const
{
    const llvm::ConstantExpr *constant =
        llvm::dyn_cast<llvm::ConstantExpr>(mConstant);

    if (!constant)
        return false;

    return constant->getOpcode() == llvm::Instruction::GetElementPtr;
}

Value *
Constant::toModifiableValue() const
{
    if (isAPInt())
        return new Integer::Container(getAPInt());
    else
        CANAL_DIE();
}

Constant *
Constant::clone() const
{
    return new Constant(*this);
}

Constant *
Constant::cloneCleaned() const
{
    return new Constant();
}

bool
Constant::operator==(const Value &value) const
{
    // Check if rhs has the same type.
    const Constant *constant = dynCast<const Constant*>(&value);
    if (!constant)
        return false;

    return mConstant == constant->mConstant;
}

size_t
Constant::memoryUsage() const
{
    return sizeof(Constant);
}

std::string
Constant::toString() const
{
    std::stringstream ss;
    ss << "constant " << Canal::toString(*mConstant) << std::endl;
    return ss.str();
}

} // namespace Canal
