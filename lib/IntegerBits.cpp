#include "IntegerBits.h"
#include "Constant.h"
#include "Utils.h"
#include <sstream>

namespace Canal {
namespace Integer {

Bits::Bits(unsigned numBits) : mBits0(numBits, 0), mBits1(numBits, 0)
{
}

Bits::Bits(const llvm::APInt &number) : mBits0(~number), mBits1(number)
{
}

Bits *
Bits::clone() const
{
    return new Bits(*this);
}

bool
Bits::operator==(const Value& value) const
{
    const Bits *other = dynamic_cast<const Bits*>(&value);
    if (!other)
        return false;
    return mBits0 == other->mBits0 && mBits1 == other->mBits1;
}

void
Bits::merge(const Value &value)
{
    const Constant *constant = dynamic_cast<const Constant*>(&value);
    if (constant)
    {
        CANAL_ASSERT(constant->isAPInt());
        mBits1 |= constant->getAPInt();
        return;
    }

    const Bits &bits = dynamic_cast<const Bits&>(value);
    mBits0 |= bits.mBits0;
    mBits1 |= bits.mBits1;
}

size_t
Bits::memoryUsage() const
{
    return sizeof(Bits);
}

std::string
Bits::toString() const
{
    std::stringstream ss;
    ss << "Integer::Bits: ";
    for (int pos = 0; pos < mBits0.getBitWidth(); ++pos)
    {
        switch (getBitValue(pos))
        {
        case -1: ss << "_"; break;
        case  0: ss << "0"; break;
        case  1: ss << "1"; break;
        case  2: ss << "2"; break;
        default: CANAL_DIE();
        }
    }
    return ss.str();
}

static void
applyBitOperation(Bits &result, const Value &a, const Value &b, int(*operation)(int,int))
{
    const Bits &aa = dynamic_cast<const Bits&>(a),
        &bb = dynamic_cast<const Bits&>(b);
    CANAL_ASSERT(result.getBitWidth() == aa.getBitWidth() &&
                 aa.getBitWidth() == bb.getBitWidth());
    for (int pos = 0; pos < aa.getBitWidth(); ++pos)
    {
        result.setBitValue(pos, operation(aa.getBitValue(pos),
                                          bb.getBitValue(pos)));
    }
}

// First number in a pair is mBits1, second is mBits0
// 00 and 00 = 00
// 00 and 01 = 01
// 00 and 10 = 00
// 00 and 11 = 11
// 10 and 01 = 01
// 10 and 10 = 10
// 10 and 11 = 11
// 01 and 01 = 01
// 01 and 11 = 01
// 11 and 11 = 11
static int
bitAnd(int valueA, int valueB)
{
    if (valueA == 0 || valueB == 0)
        return 0;
    else if (valueA == 2 || valueB == 2)
        return 2;
    else
        return (valueA == -1 || valueB == -1) ? -1 : 1;
}

void
Bits::and_(const Value &a, const Value &b)
{
    applyBitOperation(*this, a, b, bitAnd);
}

// First number in a pair is mBits1, second is mBits0
// 00 or 00 = 00
// 00 or 01 = 00
// 00 or 10 = 10
// 00 or 11 = 11
// 10 or 01 = 10
// 10 or 10 = 10
// 10 or 11 = 10
// 01 or 01 = 01
// 01 or 11 = 11
// 11 or 11 = 11
static int
bitOr(int valueA, int valueB)
{
    if ((valueA == 0 || valueA == 1) && (valueB == 0 || valueB == 1))
        return (valueA || valueB) ? 1 : 0;
    else if (valueA == 2 || valueB == 2)
        return 2;
    else
        return (valueA == 1 || valueB == 1) ? 1 : -1;
}

void
Bits::or_(const Value &a, const Value &b)
{
    applyBitOperation(*this, a, b, bitOr);
}

// First number in a pair is mBits1, second is mBits0
// 00 xor 00 = 00
// 00 xor 01 = 00
// 00 xor 10 = 10
// 00 xor 11 = 11
// 10 xor 01 = 10
// 10 xor 10 = 01
// 10 xor 11 = 11
// 01 xor 01 = 01
// 01 xor 11 = 11
// 11 xor 11 = 11
static int
bitXor(int valueA, int valueB)
{
    if ((valueA == 0 || valueA == 1) && (valueB == 0 || valueB == 1))
        return (valueA xor valueB) ? 1 : 0;
    else if (valueA == 2 || valueB == 2)
        return 2;
    else
        return (valueA == 1 || valueB == 1) ? 1 : -1;
}

void
Bits::xor_(const Value &a, const Value &b)
{
    applyBitOperation(*this, a, b, bitXor);
}

float
Bits::accuracy() const
{
    int variableBits = 0;
    for (int pos = 0; pos < getBitWidth(); ++pos)
    {
        if (getBitValue(pos) == 2)
            ++variableBits;
    }

    return 1.0 - (variableBits / (float)getBitWidth());
}

bool
Bits::isBottom() const
{
    return mBits0 == 0 && mBits1 == 0;
}

void
Bits::setTop()
{
    mBits0 = mBits1 = ~0;
}

int
Bits::getBitValue(unsigned pos) const
{
    llvm::APInt bit(llvm::APInt::getOneBitSet(mBits0.getBitWidth(), pos));
    if ((mBits1 & bit).getBoolValue())
        return (mBits0 & bit).getBoolValue() ? 2 : 1;
    else
        return (mBits0 & bit).getBoolValue() ? 0 : -1;
}

void
Bits::setBitValue(unsigned pos, int value)
{
    llvm::APInt bit(llvm::APInt::getOneBitSet(mBits0.getBitWidth(), pos));
    switch (value)
    {
    case -1:
        mBits0 &= ~bit;
        mBits1 &= ~bit;
        break;
    case 0:
        mBits0 |= bit;
        mBits1 &= ~bit;
        break;
    case 1:
        mBits0 &= ~bit;
        mBits1 |= bit;
        break;
    case 2:
        mBits0 |= bit;
        mBits1 |= bit;
        break;
    default:
        CANAL_DIE();
    }
}

} // namespace Integer
} // namespace Canal
