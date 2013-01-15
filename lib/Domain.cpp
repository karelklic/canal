#include "Domain.h"
#include "Utils.h"
#include "WideningDataInterface.h"
#include "Environment.h"
#include <typeinfo>

namespace Canal {

Domain::Domain(const Environment &environment,
               enum DomainKind kind)
    : mEnvironment(environment),
      mKind(kind),
      mWideningData(NULL)
{
}

Domain::Domain(const Domain &value)
    : SharedData(value),
      mEnvironment(value.mEnvironment),
      mKind(value.mKind),
      mWideningData(value.mWideningData)
{
    if (mWideningData)
        mWideningData = mWideningData->clone();
}

Domain::~Domain()
{
    delete mWideningData;
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

float
Domain::accuracy() const
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::add(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fadd(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::sub(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fsub(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::mul(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fmul(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::udiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::sdiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fdiv(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::urem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::srem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::frem(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::shl(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::lshr(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::ashr(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::and_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::or_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::xor_(const Domain &a, const Domain &b)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::icmp(const Domain &a, const Domain &b,
            llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fcmp(const Domain &a, const Domain &b,
            llvm::CmpInst::Predicate predicate)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::trunc(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::zext(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::sext(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fptrunc(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fpext(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fptoui(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::fptosi(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::uitofp(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::sitofp(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::insertelement(const Domain &array,
                      const Domain &element,
                      const Domain &index)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::shufflevector(const Domain &v1,
                      const Domain &v2,
                      const std::vector<uint32_t> &mask)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::setWideningData(Widening::DataInterface *wideningData)
{
    CANAL_ASSERT_MSG(!mWideningData,
                     "Widening data set were already set.");

    mWideningData = wideningData;
}

Domain *
Domain::getValue(const Domain &offset) const
{
    Domain *result = NULL;
    std::vector<Domain*> items(getItem(offset));
    std::vector<Domain*>::const_iterator it = items.begin(),
        itend = items.end();

    for (; it != itend; ++it)
    {
        if (!result)
            result = (*it)->clone();
        else
            result->join(**it);
    }

    return result;
}

std::vector<Domain*>
Domain::getItem(const Domain &offset) const
{
    CANAL_NOT_IMPLEMENTED();
}

Domain *
Domain::getItem(uint64_t offset) const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::setItem(const Domain &offset, const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::setItem(uint64_t offset, const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Domain::isValue() const
{
    CANAL_NOT_IMPLEMENTED();
}

const llvm::Type &
Domain::getValueType() const
{
    CANAL_NOT_IMPLEMENTED();
}

bool
Domain::hasValueExactSize() const
{
    CANAL_NOT_IMPLEMENTED();
}

uint64_t
Domain::getValueExactSize()
{
    CANAL_ASSERT(hasValueExactSize());
    return mEnvironment.getTypeStoreSize(getValueType());
}

Domain *
Domain::getValueCell(uint64_t offset) const
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::mergeValueCell(uint64_t offset, const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

} // namespace Canal
