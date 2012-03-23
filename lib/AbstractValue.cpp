#include "AbstractValue.h"
#include <llvm/Support/raw_ostream.h>

bool AbstractValue::operator!=(const AbstractValue &rhs) const
{
    return !operator==(rhs);
}

void AbstractValue::add(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: add not implemented!\n";
}

void AbstractValue::sub(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: sub not implemented!\n";
}

void AbstractValue::mul(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: mul not implemented!\n";
}

void AbstractValue::div(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: div not implemented!\n";
}

void AbstractValue::rem(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: rem not implemented!\n";
}

void AbstractValue::shl(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: shl not implemented!\n";
}

void AbstractValue::shr(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: shr not implemented!\n";
}

void AbstractValue::and_(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: and not implemented!\n";
}

void AbstractValue::or_(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: or not implemented!\n";
}

void AbstractValue::xor_(const AbstractValue &a, const AbstractValue &b)
{
    llvm::errs() << "AbstractValue: xor not implemented!\n";
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream, const AbstractValue &value)
{
    value.printToStream(ostream);
    return ostream;
}
