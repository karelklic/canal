#include "ArrayStringSuffix.h"
#include "ProductVector.h"
#include "Utils.h"
#include "Environment.h"
#include "IntegerUtils.h"
#include "Constructors.h"

namespace Canal {
namespace Array {

static std::string
commonSuffix(std::string first, std::string second)
{
    if (first.length() > second.length())
        first.swap(second);

    size_t firstlen = first.length() - 1;
    size_t secondlen = second.length() - 1;

    size_t i;
    for (i = 0; i < first.length(); i++)
    {
        if (first[firstlen - i] != second[secondlen - i])
            break;
    }

    return first.substr(first.length() - i, first.length());
}

StringSuffix::StringSuffix(const Environment &environment,
                           const llvm::SequentialType &type)
    : Domain(environment, Domain::ArrayStringSuffixKind),
      mIsBottom(true),
      mType(type)
{
    const llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();
}

StringSuffix::StringSuffix(const Environment &environment,
                           const llvm::SequentialType &type,
                           std::vector<Domain *>::const_iterator begin,
                           std::vector<Domain *>::const_iterator end)
    : Domain(environment, Domain::ArrayStringSuffixKind),
      mIsBottom(true),
      mType(type)
{
    const llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();
    else
    {
        std::vector<Domain *>::const_iterator it = begin;
        for (; it != end; ++it)
        {
            if (!Integer::Utils::isConstant(**it))
                break;

            CANAL_ASSERT_MSG(8 == Integer::Utils::getBitWidth(**it),
                             "String requires 8-bit characters.");

            llvm::APInt constant;
            bool success = Integer::Utils::signedMin(**it, constant);
            CANAL_ASSERT(success);

            uint64_t c = constant.getZExtValue();
            if (c == 0 || c > 255);
                break;

            mIsBottom = false;
            mSuffix.append(1, (char)c);
        }
    }
}

StringSuffix::StringSuffix(const Environment &environment,
                           const std::string &value)
    : Domain(environment, Domain::ArrayStringSuffixKind),
      mSuffix(value),
      mIsBottom(false),
      mType(*llvm::ArrayType::get(llvm::Type::getInt8Ty(environment.getContext()),
                                  value.size()))
{
}

void
StringSuffix::strcat(const StringSuffix &source)
{
    CANAL_NOT_IMPLEMENTED();
}

StringSuffix *
StringSuffix::clone() const
{
    return new StringSuffix(*this);
}

size_t
StringSuffix::memoryUsage() const
{
    size_t size = sizeof(StringSuffix);
    size += mSuffix.size();
    return size;
}

std::string
StringSuffix::toString() const
{
    StringStream ss;
    ss << "stringSuffix ";
    
    if (isTop())
        ss << "top";

    if (isBottom())
        ss << "bottom";

    ss << "\n";
    ss << "    type " << Canal::toString(mType) << "\n";

    if (!isBottom() && !isTop())
        ss << "    \"" << mSuffix << "\"\n";

    return ss.str();
}

void
StringSuffix::setZero(const llvm::Value *place)
{
    setTop();
}

bool
StringSuffix::operator==(const Domain &value) const
{
    if (this == &value)
        return true;

    const StringSuffix &array = checkedCast<StringSuffix>(value);

    if (isBottom() != array.isBottom())
        return false;

    if (!mIsBottom && (mSuffix != array.mSuffix))
        return false;

    return true;
}

bool
StringSuffix::operator<(const Domain &value) const
{
    if (this == &value)
        return false;

    if (isTop())
        return false;

    const StringSuffix &array = checkedCast<StringSuffix>(value);

    if (array.isBottom())
        return false;

    if (!isBottom() && !array.isTop())
    {
        std::string suffix = commonSuffix(mSuffix, array.mSuffix);

        if (suffix == "" || mSuffix.length() < array.mSuffix.length())
            return false;
    }
    
    return true;
}

StringSuffix &
StringSuffix::join(const Domain &value)
{
    if (isTop())
        return *this;

    if (value.isBottom())
        return *this;

    if (value.isTop())
    {
        setTop();
        return *this;
    }

    const StringSuffix &array = checkedCast<StringSuffix>(value);

    if (isBottom())
        mSuffix = array.mSuffix;
    else
        mSuffix = commonSuffix(mSuffix, array.mSuffix);

    mIsBottom = false;
    return *this;
}

StringSuffix &
StringSuffix::meet(const Domain &value)
{
    if (isBottom())
        return *this;

    if (value.isTop())
        return *this;

    if (value.isBottom())
    {
        setBottom();
        return *this;
    }

    const StringSuffix &array = checkedCast<StringSuffix>(value);

    if (isTop())
        mSuffix = array.mSuffix;
    else if (0 == array.mSuffix.compare(0, mSuffix.length(), mSuffix))
    {
        mSuffix = array.mSuffix;
    }
    else if (0 != mSuffix.compare(0, array.mSuffix.length(), array.mSuffix))
    {
        setBottom();
    }

    return *this;
}

bool
StringSuffix::isBottom() const
{
    return mIsBottom;
}

void
StringSuffix::setBottom()
{
    mIsBottom = true;
    mSuffix = "";
}

bool
StringSuffix::isTop() const
{
    return !mIsBottom && (mSuffix == "");
}

void
StringSuffix::setTop()
{
    mIsBottom = false;
    mSuffix = "";
}

float
StringSuffix::accuracy() const
{
    return 0.0f;
}

StringSuffix &
StringSuffix::add(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::fadd(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::sub(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::fsub(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::mul(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::fmul(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::udiv(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::sdiv(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::fdiv(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::urem(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::srem(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::frem(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::shl(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::lshr(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::ashr(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::and_(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::or_(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::xor_(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::icmp(const Domain &a, const Domain &b,
                   llvm::CmpInst::Predicate predicate)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::fcmp(const Domain &a, const Domain &b,
                   llvm::CmpInst::Predicate predicate)
{
    setTop();
    return *this;
}

Domain *StringSuffix::extractelement(const Domain &index) const
{
    const llvm::Type &elementType = *mType.getElementType();
    Domain *result = mEnvironment.getConstructors().create(elementType);
    result->setTop();
    return result;
}

StringSuffix &
StringSuffix::insertelement(const Domain &array,
                            const Domain &element,
                            const Domain &index)
{
    setTop();
    return *this;
}

StringSuffix &
StringSuffix::shufflevector(const Domain &a,
                            const Domain &b,
                            const std::vector<uint32_t> &mask)
{
    setTop();
    return *this;
}

Domain *
StringSuffix::extractvalue(const std::vector<unsigned> &indices) const
{
    if (isTop())
    {
        // Cannot use const-correctness here because of differences
        // between LLVM versions.
        llvm::Type *type = (llvm::Type*)&mType;
        std::vector<unsigned>::const_iterator it = indices.begin(),
            itend = indices.end();

        for (; it != itend; ++it)
        {
            llvm::CompositeType *composite = checkedCast<llvm::CompositeType>(type);
            type = (llvm::Type*)composite->getTypeAtIndex(*it);
        }

        Domain *result = mEnvironment.getConstructors().create(*type);
        result->setTop();
        return result;
    }

    CANAL_ASSERT(indices.size() == 1);
    Domain *result = mEnvironment.getConstructors().create(*mType.getElementType());
    result->setTop();
    return result;
}

StringSuffix &
StringSuffix::insertvalue(const Domain &aggregate,
                          const Domain &element,
                          const std::vector<unsigned> &indices)
{
    setTop();
    return *this;
}

void
StringSuffix::insertvalue(const Domain &element,
                          const std::vector<unsigned> &indices)
{
    setTop();
}

Domain *
StringSuffix::load(const llvm::Type &type,
                   const std::vector<Domain*> &offsets) const
{
    if (offsets.empty())
    {
        if (&mType == &type)
            return clone();
        else
        {
            Domain *result = mEnvironment.getConstructors().create(type);
            result->setTop();
            return result;
        }
    }

    Domain *subitem = extractelement(*offsets[0]);
    Domain *result = subitem->load(type, std::vector<Domain*>(offsets.begin() + 1,
                                                              offsets.end()));

    delete subitem;
    return result;
}

StringSuffix &
StringSuffix::store(const Domain &value,
                    const std::vector<Domain*> &offsets,
                    bool overwrite)
{
    if (offsets.empty())
        return (StringSuffix&)Domain::store(value, offsets, overwrite);

    setTop();
    return *this;
}

} // namespace Array
} // namespace Canal

