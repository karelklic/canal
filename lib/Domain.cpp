#include "Domain.h"
#include "Utils.h"
#include "WideningDataInterface.h"
#include "Environment.h"
#include "Constructors.h"

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

Domain *
Domain::extractelement(const Domain &index) const
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

Domain *
Domain::extractvalue(const std::vector<unsigned> &indices) const
{
    CANAL_NOT_IMPLEMENTED();
}

Domain &
Domain::insertvalue(const Domain &aggregate,
                    const Domain &element,
                    const std::vector<unsigned> &indices)
{
    CANAL_NOT_IMPLEMENTED();
}

void
Domain::insertvalue(const Domain &element,
                    const std::vector<unsigned> &indices)
{
    CANAL_NOT_IMPLEMENTED();
}

Domain *
Domain::load(const llvm::Type &type,
             const std::vector<Domain*> &offsets) const
{
    // Default implementation.  Useful for non-array objects.
    CANAL_ASSERT(offsets.empty());
    if (&type == &getValueType())
        return clone();
    else
    {
        Domain *result = mEnvironment.getConstructors().create(type);
        result->setTop();
        return result;
    }
}

Domain &
Domain::store(const Domain &value,
              const std::vector<Domain*> &offsets,
              bool overwrite)
{
    CANAL_ASSERT(offsets.empty());
    if (&getValueType() == &value.getValueType())
    {
        if (overwrite)
            setBottom();

        join(value);
    }
    else
        setTop();

    return *this;
}

void
Domain::setWideningData(Widening::DataInterface *wideningData)
{
    CANAL_ASSERT_MSG(!mWideningData,
                     "Widening data set were already set.");

    mWideningData = wideningData;
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

bool
Domain::canDeserialize(llvm::MDNode *what, unsigned offset) const
{
    return false;
}

void
Domain::deserialize(llvm::MDNode *what, unsigned offset) {
    CANAL_NOT_IMPLEMENTED();
}

llvm::MDNode*
Domain::serialize() const {
    return NULL;
}

} // namespace Canal
