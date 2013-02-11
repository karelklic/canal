#include "ArrayStringTrie.h"
#include "Environment.h"
#include "Utils.h"

namespace Canal {
namespace Array {

StringTrie::StringTrie(const Environment &environment,
                       const llvm::SequentialType &type)
    : Domain(environment, Domain::ArrayStringTrieKind),
      mIsBottom(true),
      mType(type)
{
    const llvm::Type *int8 = llvm::Type::getInt8Ty(environment.getContext());
    if (mType.getElementType() != int8)
        setTop();
}

StringTrie *
StringTrie::clone() const
{
    CANAL_NOT_IMPLEMENTED();
}

size_t
StringTrie::memoryUsage() const
{
    CANAL_NOT_IMPLEMENTED();
}

std::string
StringTrie::toString() const
{
    CANAL_NOT_IMPLEMENTED();
}

void
StringTrie::setZero(const llvm::Value *place)
{
    CANAL_NOT_IMPLEMENTED();
}

bool
StringTrie::operator==(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

bool StringTrie::operator<(const Domain &value) const
{
    CANAL_NOT_IMPLEMENTED();
}

StringTrie &
StringTrie::join(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

StringTrie &
StringTrie::meet(const Domain &value)
{
    CANAL_NOT_IMPLEMENTED();
}

bool
StringTrie::isBottom() const
{
    return mIsBottom;
}

void StringTrie::setBottom()
{
    mIsBottom = true;
    mRoot = NULL;
}

bool
StringTrie::isTop() const
{
    return !mIsBottom && (mRoot == NULL);
}

void StringTrie::setTop()
{
    mIsBottom = false;
    mRoot = NULL;
}

} // namespace Array
} // namespace Canal
