#include "Domain.h"
#include "Utils.h"
#include <typeinfo>
#include <llvm/Support/raw_ostream.h>

namespace Canal {

Domain::Domain(const Environment &environment)
    : mEnvironment(environment)
{
}

bool
Domain::operator!=(const Domain &rhs) const
{
    return !operator==(rhs);
}

void
Domain::merge(const Domain &v)
{
    CANAL_DIE_MSG("merge not implemented for "
                  << typeid(*this).name() << " and "
                  << typeid(v).name());
}

void
Domain::add(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fadd(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::sub(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fsub(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::mul(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fmul(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::udiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::sdiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fdiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::urem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::srem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::frem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::shl(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::lshr(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::ashr(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::and_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::or_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::xor_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::icmp(const Domain &a, const Domain &b,
            llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fcmp(const Domain &a, const Domain &b,
            llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::trunc(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::zext(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::sext(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fptrunc(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fpext(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fptoui(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::fptosi(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::uitofp(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::sitofp(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

llvm::raw_ostream&
operator<<(llvm::raw_ostream& ostream, const Domain &value)
{
    ostream << value.toString();
    return ostream;
}


float
AccuracyDomain::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

bool
AccuracyDomain::isBottom() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
AccuracyDomain::setBottom()
{
    CANAL_NOT_IMPLEMENTED();
}

bool
AccuracyDomain::isTop() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
AccuracyDomain::setTop()
{
    CANAL_NOT_IMPLEMENTED();
}

bool
VariablePrecisionDomain::limitMemoryUsage(size_t size)
{
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
