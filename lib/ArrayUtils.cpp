#include "ArrayUtils.h"
#include "ArrayStringPrefix.h"
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
    }
}

} // namespace Utils
} // namespace Array
} // namespace Canal
