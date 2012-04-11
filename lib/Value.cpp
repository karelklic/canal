#include "Value.h"
#include "Utils.h"
#include <llvm/Support/raw_ostream.h>

namespace Canal {

bool Value::operator!=(const Value &rhs) const
{
    return !operator==(rhs);
}

void Value::merge(const Value &v)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::add(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::sub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::mul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::div(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::rem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::shl(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::shr(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::and_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::or_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::xor_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream, const Value &value)
{
    value.printToStream(ostream);
    return ostream;
}

} // namespace Canal
