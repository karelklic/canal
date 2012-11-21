#include "Domain.h"
#include "Utils.h"
#include "WideningDataInterface.h"
#include <typeinfo>

namespace Canal {

Domain::Domain(const Environment &environment)
    : mEnvironment(environment),
      mWideningData(NULL)
{
}

Domain::Domain(const Domain &value)
    : SharedData(value),
      mEnvironment(value.mEnvironment),
      mWideningData(value.mWideningData)
{
    if (mWideningData)
        mWideningData = mWideningData->clone();
}

Domain::~Domain()
{
    delete mWideningData;
}

void
Domain::setWideningData(Widening::DataInterface *wideningData)
{
    CANAL_ASSERT_MSG(!mWideningData,
                     "Widening data set were already set.");

    mWideningData = wideningData;
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

float
Domain::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Domain::isBottom() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::setBottom()
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Domain::isTop() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::setTop()
{
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
