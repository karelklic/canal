#include "Value.h"
#include "Utils.h"
#include <typeinfo>
#include <llvm/Support/raw_ostream.h>

namespace Canal {

bool
Value::operator!=(const Value &rhs) const
{
    return !operator==(rhs);
}

void
Value::merge(const Value &v)
{
    CANAL_DIE_MSG("merge not implemented for "
                  << typeid(*this).name() << " and "
                  << typeid(v).name());
}

void
Value::add(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::fadd(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::sub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::fsub(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::mul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::fmul(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::udiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::sdiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::fdiv(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::urem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::srem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::frem(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::shl(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::lshr(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::ashr(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::and_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::or_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::xor_(const Value &a, const Value &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::icmp(const Value &a, const Value &b,
            llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::fcmp(const Value &a, const Value &b,
            llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::trunc(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::zext(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::sext(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::fptrunc(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::Value::fpext(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::fptoui(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::fptosi(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::uitofp(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Value::Value::sitofp(const Value &value)
{
    CANAL_NOT_IMPLEMENTED();
}

llvm::raw_ostream&
operator<<(llvm::raw_ostream& ostream, const Value &value)
{
    ostream << value.toString();
    return ostream;
}


float
AccuracyValue::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

bool
AccuracyValue::isBottom() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
AccuracyValue::setBottom()
{
    CANAL_NOT_IMPLEMENTED();
}

bool
AccuracyValue::isTop() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
AccuracyValue::setTop()
{
    CANAL_NOT_IMPLEMENTED();
}

bool
VariablePrecisionValue::limitMemoryUsage(size_t size)
{
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
