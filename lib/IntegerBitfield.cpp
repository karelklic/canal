#include "IntegerBitfield.h"
#include "Utils.h"
#include "APIntUtils.h"
#include "FloatInterval.h"

namespace Canal {
namespace Integer {

Bitfield::Bitfield(const Environment &environment,
                   unsigned bitWidth)
    : Domain(environment),
      mZeroes(bitWidth, 0),
      mOnes(bitWidth, 0)
{
}

Bitfield::Bitfield(const Environment &environment,
                   const llvm::APInt &number)
    : Domain(environment),
      mZeroes(~number),
      mOnes(number)
{
}

Bitfield::Bitfield(const Bitfield &value)
    : Domain(value),
      mZeroes(value.mZeroes),
      mOnes(value.mOnes)
{
}

int
Bitfield::getBitValue(unsigned pos) const
{
    llvm::APInt bit(APIntUtils::getOneBitSet(mZeroes.getBitWidth(), pos));
    if ((mOnes & bit).getBoolValue())
        return (mZeroes & bit).getBoolValue() ? 2 : 1;
    else
        return (mZeroes & bit).getBoolValue() ? 0 : -1;
}

void
Bitfield::setBitValue(unsigned pos, int value)
{
    llvm::APInt bit(APIntUtils::getOneBitSet(mZeroes.getBitWidth(), pos));
    switch (value)
    {
    case -1:
        mZeroes &= ~bit;
        mOnes &= ~bit;
        break;
    case 0:
        mZeroes |= bit;
        mOnes &= ~bit;
        break;
    case 1:
        mZeroes &= ~bit;
        mOnes |= bit;
        break;
    case 2:
        mZeroes |= bit;
        mOnes |= bit;
        break;
    default:
        CANAL_DIE();
    }
}

static inline
void resizeAndClearResult(llvm::APInt &result, const unsigned bitWidth)
{
    result = llvm::APInt(bitWidth, 0, false);
}

bool
Bitfield::signedMin(llvm::APInt &result) const
{

    resizeAndClearResult(result, getBitWidth());

    for (unsigned i = 0; i < getBitWidth(); ++i)
    {
        switch (getBitValue(i))
        {
        case -1:
            // A bit has unknown (undefined) value.
            return false;
        case 0:
            break;
        case 1:
            APIntUtils::setBit(result, i);
            break;
        case 2:
            // If sign bit, set to 1, otherwise to 0
            // TTTTTTTTTT -> 100000000
            if (i == getBitWidth() - 1)
                APIntUtils::setBit(result, i);
            break;
        default:
            CANAL_DIE();
        }
    }

    return true;
}

bool
Bitfield::signedMax(llvm::APInt &result) const
{
    resizeAndClearResult(result, getBitWidth());

    for (unsigned i = 0; i < getBitWidth(); ++i)
    {
        switch (getBitValue(i))
        {
        case -1:
            // A bit has unknown (undefined) value.
            return false;
        case 0:
            break;
        case 1:
            APIntUtils::setBit(result, i);
            break;
        case 2:
            // If sign bit, set to 0, otherwise to 1
            // TTTTTTTTTTT -> 01111111111
            if (i != getBitWidth() - 1)
                APIntUtils::setBit(result, i);
            break;
        default:
            CANAL_DIE();
        }
    }

    return true;
}

bool
Bitfield::unsignedMin(llvm::APInt &result) const
{
    resizeAndClearResult(result, getBitWidth());

    for (unsigned i = 0; i < getBitWidth(); ++i)
    {
        switch (getBitValue(i))
        {
        case -1:
            // A bit has unknown (undefined) value.
            return false;
        case 0:
        case 2: // We choose 0 when both 0 and 1 are available...
            break;
        case 1:
            APIntUtils::setBit(result, i);
            break;
        default:
            CANAL_DIE();
        }
    }

    return true;
}

bool
Bitfield::unsignedMax(llvm::APInt &result) const
{
    resizeAndClearResult(result, getBitWidth());

    for (unsigned i = 0; i < getBitWidth(); ++i)
    {
        switch (getBitValue(i))
        {
        case -1:
            // A bit has unknown (undefined) value.
            return false;
        case 0:
            break;
        case 1:
        case 2: // We choose 1 when both 0 and 1 are available...
            APIntUtils::setBit(result, i);
            break;
        default:
            CANAL_DIE();
        }
    }

    return true;
}

bool
Bitfield::isSingleValue() const
{
    return ~(mZeroes ^ mOnes) == 0;
}

Bitfield *
Bitfield::clone() const
{
    return new Bitfield(*this);
}

bool
Bitfield::operator==(const Domain& value) const
{
    if (&value == this) return true;
    const Bitfield *other = dynCast<const Bitfield*>(&value);
    if (!other)
        return false;
    return mZeroes == other->mZeroes && mOnes == other->mOnes;
}

void
Bitfield::merge(const Domain &value)
{
    const Bitfield &bits = dynCast<const Bitfield&>(value);
    mZeroes |= bits.mZeroes;
    mOnes |= bits.mOnes;
}

size_t
Bitfield::memoryUsage() const
{
    return sizeof(Bitfield);
}

std::string
Bitfield::toString() const
{
    StringStream ss;
    ss << "bitfield ";
    for (int pos = mZeroes.getBitWidth() - 1; pos >= 0; --pos)
    {
        switch (getBitValue(pos))
        {
        case -1: ss << "_"; break;
        case  0: ss << "0"; break;
        case  1: ss << "1"; break;
        case  2: ss << "T"; break;
        default: CANAL_DIE();
        }
    }
    ss << "\n";
    return ss.str();
}

void
Bitfield::setZero(const llvm::Value *place)
{
    mZeroes = ~0;
    mOnes = 0;
}

void
Bitfield::add(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::sub(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::mul(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::udiv(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::sdiv(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::urem(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::srem(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::shl(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::lshr(const Domain &a, const Domain &b)
{
    setTop();
}

void
Bitfield::ashr(const Domain &a, const Domain &b)
{
    setTop();
}

static void
bitOperation(Bitfield &result,
             const Domain &a,
             const Domain &b,
             int(*operation)(int,int))
{
    const Bitfield &aa = dynCast<const Bitfield&>(a),
        &bb = dynCast<const Bitfield&>(b);

    CANAL_ASSERT(aa.getBitWidth() == bb.getBitWidth() &&
                 result.getBitWidth() == aa.getBitWidth());

    for (unsigned pos = 0; pos < aa.getBitWidth(); ++pos)
    {
        result.setBitValue(pos, operation(aa.getBitValue(pos),
                                          bb.getBitValue(pos)));
    }
}

// First number in a pair is mOnes, second is mZeroes
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
Bitfield::and_(const Domain &a, const Domain &b)
{
    bitOperation(*this, a, b, bitAnd);
}

// First number in a pair is mOnes, second is mZeroes
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
Bitfield::or_(const Domain &a, const Domain &b)
{
    bitOperation(*this, a, b, bitOr);
}

// First number in a pair is mOnes, second is mZeroes
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
Bitfield::xor_(const Domain &a, const Domain &b)
{
    bitOperation(*this, a, b, bitXor);
}

// -1 if a < b, 0 if a == b, 1 if a > b, 2 if unknown
static int
compare(const Bitfield &a,
        const Bitfield &b,
        bool signed_,
        bool equality = false)
{
    //Possible todo - TT >= 10 if signed
    bool first = true;
    for (int pos = a.getBitWidth() - 1; pos >= 0; --pos)
    {
        int i = a.getBitValue(pos);
        int j = b.getBitValue(pos);
        if (i == -1 || i == 2 || j == -1 || j == 2)
        {
            if (equality && !first)
            {
                // If comparing with equality, then if one value is
                // set and the other one is TOP: 0<=TOP and 1>=TOP ->
                // return 0<TOP and 1>TOP, and comparison operator
                // will handle the result correctly.
                if ((i == -1 || i == 2) && (j == -1 || j == 2))
                    return 2; // If both values are top

                if (i == 0 || i == 1)
                    return (i ? -1 : 1); // If first value is set
                else
                    return (j ? -1 : 1); // If second value is set
            }

            return 2;
        }

        if (i != j)
        {
            // Inequality
            if (i < j) // a < b
                return ((first && signed_) ? 1 : -1);

            // a > b
            return 1;
        }
        first = false;
    }

    return 0;
}

//0 if equal, 1 if not equal, -1 if unknown
static int
compareEqual(const Bitfield &a, const Bitfield &b)
{
    bool wasTop = false;
    for (int pos = a.getBitWidth() - 1; pos >= 0; --pos)
    {
        int i = a.getBitValue(pos);
        int j = b.getBitValue(pos);
        if (i == -1 || i == 2 || j == -1 || j == 2)
        {
            // You found unknown value -> the bitfields are not equal
            wasTop = true;
            continue;
        }

        if (i != j)
            return 1;
    }

    return (wasTop ? -1 : 0);
}

void
Bitfield::icmp(const Domain &a, const Domain &b,
               llvm::CmpInst::Predicate predicate)
{

    const Bitfield &aa = dynCast<const Bitfield&>(a),
            &bb = dynCast<const Bitfield&>(b);

    if (aa.isTop() || bb.isTop())
    {
        setTop(); // Could be both
        return;
    }

    if (aa.isBottom() || bb.isBottom())
    {
        setBottom(); // Undefined
        return;
    }
    CANAL_ASSERT(aa.getBitWidth() == bb.getBitWidth());

    switch (predicate)
    {
    case llvm::CmpInst::ICMP_EQ:  // equal
        // If it is the same object or all bits are known in
        // both Bitfield and are the same, the result is 1.
        // If there is at least one known bit in both Bitfields,
        // that differs, the result is 0.
        // Otherwise the result is the top value (both 0 and 1).
        if (&a == &b)
        { // Same object
            mZeroes = ~1;
            mOnes = 1;
            break;
        }

        switch (compareEqual(aa, bb))
        {
        case 0:  mZeroes = ~1; mOnes = 1; break;
        case 1:  mZeroes = ~0; mOnes = 0; break;
        default: setTop(); break;
        }

        break;
    case llvm::CmpInst::ICMP_NE:  // not equal
        // If it is the same object or all bits are known in
        // both Bitfield and are the same, the result is 0.
        // If there is at least one known bit in both Bitfields,
        // that differs, the result is 1.
        // Otherwise the result is the top value (both 0 and 1).
        if (&a == &b)
        { //Same object
            mZeroes = ~0;
            mOnes = 0;
            break;
        }

        switch (compareEqual(aa, bb))
        {
        case 0:  mZeroes = ~0; mOnes = 0; break;
        case 1:  mZeroes = ~1; mOnes = 1; break;
        default: setTop(); break;
        }

        break;
    case llvm::CmpInst::ICMP_UGT: // unsigned greater than
        switch (compare(aa, bb, false))
        {
        case 1:  mZeroes = ~1; mOnes = 1; break;
        case 2:  setTop(); break;
        default: mZeroes = ~0; mOnes = 0;
        }

        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        if (&a == &b)
        { //Same object
            mZeroes = ~1;
            mOnes = 1;
            break;
        }

        switch (compare(aa, bb, false, true))
        {
        case 0:
        case 1:  mZeroes = ~1; mOnes = 1; break;
        case 2:  setTop(); break;
        default: mZeroes = ~0; mOnes = 0;
        }

        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        switch (compare(aa, bb, false))
        {
        case -1: mZeroes = ~1; mOnes = 1; break;
        case 2:  setTop(); break;
        default: mZeroes = ~0; mOnes = 0;
        }

        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        if (&a == &b)
        { //Same object
            mZeroes = ~1;
            mOnes = 1;
            break;
        }

        switch (compare(aa, bb, false, true))
        {
        case 0:
        case -1: mZeroes = ~1; mOnes = 1; break;
        case 2:  setTop(); break;
        default: mZeroes = ~0; mOnes = 0;
        }

        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than
        switch (compare(aa, bb, true))
        {
        case 1:  mZeroes = ~1; mOnes = 1; break;
        case 2:  setTop(); break;
        default: mZeroes = ~0; mOnes = 0;
        }

        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        if (&a == &b)
        { //Same object
            mZeroes = ~1;
            mOnes = 1;
            break;
        }

        switch (compare(aa, bb, true, true))
        {
        case 0:
        case 1:  mZeroes = ~1; mOnes = 1; break;
        case 2:  setTop(); break;
        default: mZeroes = ~0; mOnes = 0;
        }

        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        switch (compare(aa, bb, true))
        {
        case -1: mZeroes = ~1; mOnes = 1; break;
        case 2:  setTop(); break;
        default: mZeroes = ~0; mOnes = 0;
        }

        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        if (&a == &b)
        { //Same object
            mZeroes = ~1;
            mOnes = 1;
            break;
        }

        switch (compare(aa, bb, true, true))
        {
        case 0:
        case -1: mZeroes = ~1; mOnes = 1; break;
        case 2:  setTop(); break;
        default: mZeroes = ~0; mOnes = 0;
        }

        break;
    default:
        CANAL_DIE();
    }
}

void
Bitfield::fcmp(const Domain &a, const Domain &b,
               llvm::CmpInst::Predicate predicate)
{
    const Float::Interval &aa = dynCast<const Float::Interval&>(a),
        &bb = dynCast<const Float::Interval&>(b);

    int result = aa.compare(bb, predicate);
    switch (result)
    {
    case -1:
        setBottom();
        break;
    case 0:
        mZeroes = 1;
        mOnes = 0;
        break;
    case 1:
        mZeroes = 0;
        mOnes = 1;
        break;
    case 2:
        setTop();
        break;
    default:
        CANAL_DIE();
    }
}

void
Bitfield::trunc(const Domain &value)
{
    const Bitfield &bitfield = dynCast<const Bitfield&>(value);
    mZeroes = APIntUtils::trunc(bitfield.mZeroes, getBitWidth());
    mOnes = APIntUtils::trunc(bitfield.mOnes, getBitWidth());
}

void
Bitfield::zext(const Domain &value)
{
    const Bitfield &bitfield = dynCast<const Bitfield&>(value);
    mZeroes = APIntUtils::zext(bitfield.mZeroes, getBitWidth());
    mOnes = APIntUtils::zext(bitfield.mOnes, getBitWidth());

    for (unsigned i = bitfield.mZeroes.getBitWidth(); i < getBitWidth(); ++i)
        APIntUtils::setBit(mZeroes, i);
}

void
Bitfield::sext(const Domain &value)
{
    const Bitfield &bitfield = dynCast<const Bitfield&>(value);
    mZeroes = APIntUtils::sext(bitfield.mZeroes, getBitWidth());
    mOnes = APIntUtils::sext(bitfield.mOnes, getBitWidth());
}

void
Bitfield::fptoui(const Domain &value)
{
    setTop();
}

void
Bitfield::fptosi(const Domain &value)
{
    setTop();
}

float
Bitfield::accuracy() const
{
    int variableBits = 0;
    for (unsigned pos = 0; pos < getBitWidth(); ++pos)
    {
        if (getBitValue(pos) == 2)
            ++variableBits;
    }

    return 1.0 - (variableBits / (float)getBitWidth());
}

bool
Bitfield::isBottom() const
{
    return mZeroes.countPopulation() == 0 &&
        mOnes.countPopulation() == 0;
}

void
Bitfield::setBottom()
{
    mZeroes = mOnes = 0;
}

bool
Bitfield::isTop() const
{
    return mZeroes.countPopulation() == mZeroes.getBitWidth() &&
        mOnes.countPopulation() == mOnes.getBitWidth();
}

void
Bitfield::setTop()
{
#if (LLVM_VERSION_MAJOR == 2 && LLVM_VERSION_MINOR < 9)
    // Old interface replaced in LLVM 2.9.
    mZeroes.set();
    mOnes.set();
#else
    mZeroes.setAllBits();
    mOnes.setAllBits();
#endif
}

} // namespace Integer
} // namespace Canal
