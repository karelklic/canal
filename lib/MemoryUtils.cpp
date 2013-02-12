#include "MemoryUtils.h"
#include "MemoryPointer.h"
#include "Utils.h"
#include "Environment.h"
#include "Constructors.h"
#include "IntegerUtils.h"

namespace Canal {
namespace Memory {
namespace Utils {

void
addTarget(Domain &pointer,
          const llvm::Value &target,
          Domain *offset)
{
    checkedCast<Pointer>(pointer).addTarget(target, offset);
}

Domain *
getByteOffset(std::vector<const Domain*>::const_iterator elementsBegin,
              std::vector<const Domain*>::const_iterator elementsEnd,
              const llvm::Type &type,
              const Environment &environment)
{
    const Domain *elementOffset = *elementsBegin;

    if (elementOffset->isTop())
    {
        Domain *result = environment.getConstructors().createInteger(64);
        result->setTop();
        return result;
    }
    else if (elementOffset->isBottom())
        return environment.getConstructors().createInteger(64);

    // Normalize the element offset to 64-bit.
    bool ownsElementOffset = false;

    unsigned bitWidth = Integer::Utils::getBitWidth(*elementOffset);
    CANAL_ASSERT_MSG(bitWidth <= 64,
                     "Cannot handle GetElementPtr offset"
                     " with more than 64 bits.");

    if (bitWidth < 64)
    {
        Domain *newElementOffset = environment.getConstructors().createInteger(64);
        newElementOffset->zext(*elementOffset);
        elementOffset = newElementOffset;
        ownsElementOffset = true;
    }

    const llvm::SequentialType *sequentialType =
        dynCast<llvm::SequentialType>(&type);

    if (sequentialType)
    {
        const llvm::Type &elementType = *sequentialType->getElementType();
        uint64_t elementSize = environment.getTypeStoreSize(elementType);
        llvm::APInt elementSizeConcrete(/*numBits=*/64, /*val=*/elementSize);
        Domain *elementSizeAbstract = environment.getConstructors().createInteger(elementSizeConcrete);
        Domain *base = environment.getConstructors().createInteger(64);
        base->mul(*elementSizeAbstract, *elementOffset);
        delete elementSizeAbstract;

        if (ownsElementOffset)
            delete elementOffset;

        if (elementsBegin + 1 != elementsEnd)
        {
            Domain *subcomponents = getByteOffset(elementsBegin + 1,
                                                  elementsEnd,
                                                  elementType,
                                                  environment);

            Domain *result = environment.getConstructors().createInteger(64);
            result->add(*base, *subcomponents);
            delete base;
            delete subcomponents;
            return result;
        }
        else
            return base;
    }
    else
    {
        const llvm::StructType &structType = checkedCast<llvm::StructType>(type);
        llvm::APInt min, max;

        bool success = Integer::Utils::unsignedMin(*elementOffset, min);
        CANAL_ASSERT(success);
        success = Integer::Utils::unsignedMax(*elementOffset, max);
        CANAL_ASSERT(success);

        llvm::APInt concreteZero(/*numBits=*/64, /*val=*/0L);
        Domain *baseLoop = environment.getConstructors().createInteger(concreteZero);
        Domain *base = environment.getConstructors().createInteger(64);
        Domain *subcomponents = environment.getConstructors().createInteger(64);

        for (unsigned i = 0; i < structType.getNumElements() && i < max.getZExtValue(); ++i)
        {
            const llvm::Type &elementType = *structType.getElementType(i);
            uint64_t elementSize = environment.getTypeStoreSize(elementType);
            llvm::APInt elementSizeConcrete(/*numBits=*/64, /*val=*/elementSize);
            Domain *elementSizeAbstract = environment.getConstructors().createInteger(elementSizeConcrete);

            // baseLoop += element_size
            Domain *oldBaseLoop = baseLoop;
            baseLoop = environment.getConstructors().createInteger(64);
            baseLoop->add(*oldBaseLoop, *elementSizeAbstract);
            delete oldBaseLoop;

            if (i >= min.getZExtValue())
            {
                base->join(*baseLoop);

                if (elementsBegin + 1 != elementsEnd)
                {
                    Domain *subcomponentsElement = getByteOffset(elementsBegin + 1,
                                                                 elementsEnd,
                                                                 elementType,
                                                                 environment);

                    subcomponents->join(*subcomponentsElement);
                    delete subcomponentsElement;
                }
            }

            delete elementSizeAbstract;
        }

        delete baseLoop;
        if (ownsElementOffset)
            delete elementOffset;

        if (elementsBegin + 1 != elementsEnd)
        {
            Domain *result = environment.getConstructors().createInteger(64);
            result->add(*base, *subcomponents);
            delete base;
            delete subcomponents;
            return result;
        }
        else
        {
            delete subcomponents;
            return base;
        }
    }
}

bool
isGlobal(const llvm::Value &place)
{
    bool isGlobal = llvm::isa<llvm::GlobalVariable>(place);
    CANAL_ASSERT(isGlobal ||
                 llvm::isa<llvm::Instruction>(place) ||
                 llvm::isa<llvm::Argument>(place));

    return isGlobal;
}


} // namespace Utils
} // namespace Memory
} // namespace Canal
