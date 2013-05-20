#include "ArrayUtils.h"
#include "ArrayStringPrefix.h"
#include "ArrayStringSuffix.h"
#include "ArrayStringTrie.h"
#include "ProductVector.h"
#include "Utils.h"

namespace Canal {
namespace Array {
namespace Utils {

void
strcat(Domain &destination, const Domain &source)
{
    Product::Vector &destVector = checkedCast<Product::Vector>(destination);
    const Product::Vector &srcVector = checkedCast<Product::Vector>(source);

    std::vector<Domain*>::iterator destIt = destVector.mValues.begin(),
        destItEnd = destVector.mValues.end();

    std::vector<Domain*>::const_iterator srcIt = srcVector.mValues.begin();

    for (; destIt != destItEnd; ++destIt, ++srcIt)
    {
        StringPrefix *destPrefix = castOrNull<Array::StringPrefix>(*destIt);
        if (!destPrefix)
        {
            (*destIt)->setTop();
            continue;
        }

        const StringPrefix &srcPrefix = checkedCast<Array::StringPrefix>(**srcIt);
        destPrefix->strcat(srcPrefix);

        StringSuffix *destSuffix = castOrNull<Array::StringSuffix>(*destIt);
        if (!destSuffix)
        {
            (*destIt)->setTop();
            continue;
        }

        const StringSuffix &srcSuffix = checkedCast<Array::StringSuffix>(**srcIt);
        destSuffix->strcat(srcSuffix);

        StringTrie *destTrie = castOrNull<Array::StringTrie>(*destIt);
        if (!destTrie)
        {
            (*destIt)->setTop();
            continue;
        }

        const StringTrie &srcTrie = checkedCast<Array::StringTrie>(**srcIt);
        destTrie->strcat(srcTrie);
    }
}

} // namespace Utils
} // namespace Array
} // namespace Canal
