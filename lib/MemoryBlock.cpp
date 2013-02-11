#include "MemoryBlock.h"
#include "Utils.h"
#include "Domain.h"
#include "Environment.h"
#include "Constructors.h"

namespace Canal {
namespace Memory {

Block::Block(MemoryType memoryType, Domain *mainValue)
    : mMainValue(mainValue), mMemoryType(memoryType)
{
    CANAL_ASSERT(mainValue);
}

Block::Block(const Block &block)
    : mMainValue(block.mMainValue), mMemoryType(block.mMemoryType)
{
    mMainValue = mMainValue->clone();
}

Block::~Block()
{
    delete mMainValue;
}

bool
Block::operator==(const Block &block) const
{
    return mMemoryType == block.mMemoryType &&
        *mMainValue == *block.mMainValue;
}

bool
Block::operator<(const Block &block) const
{
    return mMemoryType == block.mMemoryType &&
        *mMainValue < *block.mMainValue;
}

size_t
Block::memoryUsage() const
{
    return sizeof(Block) + mMainValue->memoryUsage();
}

std::string
Block::toString() const
{
    return mMainValue->toString();
}

void
Block::join(const Block &block)
{
    CANAL_ASSERT(mMemoryType == block.mMemoryType);
    mMainValue->join(*block.mMainValue);
}

Domain *
Block::load(const llvm::Type &type,
            const Domain &offset) const
{
    // TODO: increase precision.
    Domain *result = mMainValue->getEnvironment().getConstructors().create(type);
    result->setTop();
    return result;
}

void
Block::store(const Domain &value,
             const Domain &offset,
             bool isSingleTarget)
{
    // TODO: increase precision.
    mMainValue->setTop();
}

} // namespace Memory
} // namespace Canal
