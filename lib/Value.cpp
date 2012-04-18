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

void Value::fadd(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::sub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::fsub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::mul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::fmul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::udiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::sdiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::fdiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::urem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::srem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::frem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::shl(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::lshr(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void Value::ashr(const Value &a, const Value &b)
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
