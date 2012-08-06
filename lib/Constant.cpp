#include "Constant.h"
#include "IntegerContainer.h"
#include "Utils.h"
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
    return llvm::cast<llvm::ConstantInt>(mConstant)->getValue();
}

bool
Constant::isGetElementPtr() const
{
    const llvm::ConstantExpr *constant = llvm::cast_or_null<llvm::ConstantExpr>(mConstant);
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

bool
Constant::operator==(const Value &value) const
{
    // Check if rhs has the same type.
    const Constant *constant = dynamic_cast<const Constant*>(&value);
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
    std::string s;
    llvm::raw_string_ostream os(s);
    os << "Constant: " << *mConstant;
    os.flush();
    return s;
}

} // namespace Canal
