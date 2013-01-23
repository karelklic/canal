#include "IntegerBitfield.h"
#include "Utils.h"
#include "APIntUtils.h"
#include "FloatInterval.h"
#include "Environment.h"
#include <queue>

namespace Canal {
namespace Integer {

Bitfield::Bitfield(const Environment &environment,
                   unsigned bitWidth)
    : Domain(environment, Domain::IntegerBitfieldKind),
      mZeroes(bitWidth, 0),
      mOnes(bitWidth, 0)
{
}

Bitfield::Bitfield(const Environment &environment,
                   const llvm::APInt &number)
    : Domain(environment, Domain::IntegerBitfieldKind),
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

static void
resizeAndClearResult(llvm::APInt &result, const unsigned bitWidth)
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
Bitfield::isConstant() const
{
    return ~(mZeroes ^ mOnes) == 0;
}

bool
Bitfield::isTrue() const
{
    return mZeroes.getBitWidth() == 1 && mZeroes == 0 && mOnes == 1;
}

bool
Bitfield::isFalse() const
{
    return mZeroes.getBitWidth() == 1 && mZeroes == 1 && mOnes == 0;
}

Bitfield *
Bitfield::clone() const
{
    return new Bitfield(*this);
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
    ss << "integerBitfield ";
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

bool
Bitfield::operator==(const Domain& value) const
{
    if (this == &value)
        return true;

    const Bitfield &other = checkedCast<Bitfield>(value);
    return mZeroes == other.mZeroes && mOnes == other.mOnes;
}

bool
Bitfield::operator<(const Domain& value) const
{
    const Bitfield &bits = checkedCast<Bitfield>(value);
    //0 0 -> true
    //0 1 -> true
    //1 0 -> false
    //1 1 -> true
    // aka implication -> not a or b
    if (!((~mZeroes) | bits.mZeroes).isAllOnesValue()) return false;
    if (!((~mOnes) | bits.mOnes).isAllOnesValue()) return false;
    return true;
}

Bitfield &
Bitfield::join(const Domain &value)
{
    const Bitfield &bits = checkedCast<Bitfield>(value);
    mZeroes |= bits.mZeroes;
    mOnes |= bits.mOnes;
    return *this;
}

Bitfield &
Bitfield::meet(const Domain &value)
{
    const Bitfield &bits = checkedCast<Bitfield>(value);
    mZeroes &= bits.mZeroes;
    mOnes &= bits.mOnes;
    return *this;
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

Bitfield &
Bitfield::add(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

Bitfield &
Bitfield::sub(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

Bitfield &
Bitfield::mul(const Domain &a, const Domain &b)
{
    setTop();
    return *this;
}

Bitfield &
Bitfield::udiv(const Domain &a, const Domain &b)
{
    setTop();
    //When implemented, handle division by zero - see #145
    return *this;
}

Bitfield &
Bitfield::sdiv(const Domain &a, const Domain &b)
{
    setTop();
    //When implemented, handle division by zero - see #145
    return *this;
}

Bitfield &
Bitfield::urem(const Domain &a, const Domain &b)
{
    setTop();
    //When implemented, handle division by zero - see #145
    return *this;
}

Bitfield &
Bitfield::srem(const Domain &a, const Domain &b)
{
    setTop();
    //When implemented, handle division by zero - see #145
    return *this;
}

void
Bitfield::shift(const Domain &a, const Domain &b, bool right, int shiftValue)
{
    const Bitfield &aa = checkedCast<Bitfield>(a),
        &bb = checkedCast<Bitfield>(b);

    CANAL_ASSERT(aa.getBitWidth() == bb.getBitWidth() &&
            getBitWidth() == aa.getBitWidth());

    if (aa.isTop() || bb.isTop())
    {
        setTop();
        return;
    }

    if (aa.isBottom() || bb.isBottom())
    {
        setBottom();
        return;
    }

    unsigned counts[4] = {0, 0, 0, 0}; //Bottoms, zeros, ones, tops
    llvm::APInt minShift_, maxShift_;
    if (!bb.unsignedMin(minShift_) || !bb.unsignedMax(maxShift_))
        setBottom(); //If at least one bit is bottom, the result should be bottom
    if (minShift_.uge(getBitWidth())) { //You shift for more than the bitwidth -> result is 0
        mZeroes = ~0; mOnes = 0;
        return;
    }

    maxShift_ -= minShift_; //It has to be the size
    unsigned minShift = minShift_.getLimitedValue(getBitWidth()),
            maxShift = maxShift_.getLimitedValue(getBitWidth());
    counts[shiftValue + 1] = maxShift + minShift; //Put maxShift zeros from shift
    std::queue<unsigned> q; //Queue of bits to handle -> you need to handle them after minShift

    for (int i = (right ? getBitWidth() - 1 : 0); (right ? i >= 0 : i < getBitWidth()); (right ? i -- : i ++)) {
        q.push(aa.getBitValue(i) + 1); //Push this bit into queue

        if (q.size() > minShift) { //If minShift is reached, put this in for merge
            counts[q.front()] = maxShift + 1; //It can be this bit value for next maxShift + 1 bits (+1 for current)
            //Replace the current value with new one
            q.pop();
        }

        if (counts[0]) //Can be bottom
            setBitValue(i, -1); //Set bottom
        else if (counts[3] || (counts[1] && counts[2])) //Can be top or can be zero and one
            setBitValue(i, 2); //Set top
        else //Can be either zero or one
            setBitValue(i, (counts[2] ? 1 : 0));

        //Substract counts of possible bits
        counts[0] = (counts[0] ? counts[0] - 1 : 0);
        counts[1] = (counts[1] ? counts[1] - 1 : 0);
        counts[2] = (counts[2] ? counts[2] - 1 : 0);
        counts[3] = (counts[3] ? counts[3] - 1 : 0);
    }
}

Bitfield &
Bitfield::shl(const Domain &a, const Domain &b)
{
    shift(a, b, false, 0);
    return *this;
}

Bitfield &
Bitfield::lshr(const Domain &a, const Domain &b)
{
    shift(a, b, true, 0);
    return *this;
}

Bitfield &
Bitfield::ashr(const Domain &a, const Domain &b)
{
    const Bitfield &aa = checkedCast<Bitfield>(a);
    shift(a, b, true, aa.getBitValue(aa.getBitWidth() - 1));
    return *this;
}

static Bitfield &
bitOperation(Bitfield &result,
             const Domain &a,
             const Domain &b,
             int(*operation)(int,int))
{
    const Bitfield &aa = checkedCast<Bitfield>(a),
        &bb = checkedCast<Bitfield>(b);

    CANAL_ASSERT(aa.getBitWidth() == bb.getBitWidth() &&
                 result.getBitWidth() == aa.getBitWidth());

    for (unsigned pos = 0; pos < aa.getBitWidth(); ++pos)
    {
        result.setBitValue(pos, operation(aa.getBitValue(pos),
                                          bb.getBitValue(pos)));
    }

    return result;
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

Bitfield &
Bitfield::and_(const Domain &a, const Domain &b)
{
    return bitOperation(*this, a, b, bitAnd);
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

Bitfield &
Bitfield::or_(const Domain &a, const Domain &b)
{
    return bitOperation(*this, a, b, bitOr);
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

Bitfield &
Bitfield::xor_(const Domain &a, const Domain &b)
{
    return bitOperation(*this, a, b, bitXor);
}

// -1 if a < b, 0 if a == b, 1 if a > b, 2 if unknown
// and if you compare with top: 3 if !(a < b), 4 if !(a > b)
static int
compare(const Bitfield &a,
        const Bitfield &b,
        bool signed_)
{
    //Possible todo - TT >= 10 if signed
    bool first = true;
    for (int pos = a.getBitWidth() - 1; pos >= 0; --pos)
    {
        int i = a.getBitValue(pos);
        int j = b.getBitValue(pos);
        if (i == -1 || i == 2 || j == -1 || j == 2)
        {
            //If one value at least one value is top or bottom
            if ((i == -1 || i == 2) && (j == -1 || j == 2))
                return 2; // If both values are top

            // If it is first bit and comparison is signed (signed bit comparison):
            // then result is reversed
            // T = <-1; 0>
            if (first && signed_) {
                if (i == 0 || i == 1)
                    return (i ? 4 : 3); // If first value is set
                    //-1 <= T -> !(-1 > T) -> 4, 0 >= T -> !(0 < T) -> 3
                else
                    return (j ? 3 : 4); // If second value is set
                    //T >= -1 -> !(T < -1) -> 3, T <= 0 -> !(T > 0) -> 4
            }
            // Not reversed result
            // T = <0; 1>
            else {
                if (i == 0 || i == 1)
                    return (i ? 3 : 4); // If first value is set
                    //1 >= T -> !(1 < T) -> 3, 0 <= T -> !(0 > T) -> 4
                else
                    return (j ? 4 : 3); // If second value is set
                    //T <= 1 -> !(T > 1) -> 4, T >= 0 -> !(T < 0) -> 3
            }
        }

        if (i != j)
        {
            // Inequality
            // If it is first bit and comparison is signed (signed bit comparison):
            // then result is reversed
            if (i < j) // a < b
                return ((first && signed_) ? 1 : -1);

            // a > b
            return ((first && signed_) ? -1 : 1);
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

Bitfield &
Bitfield::icmp(const Domain &a, const Domain &b,
               llvm::CmpInst::Predicate predicate)
{

    const Bitfield &aa = checkedCast<Bitfield>(a),
            &bb = checkedCast<Bitfield>(b);

    //If you do not compare only one bit, TOP value means any result
    if (aa.getBitWidth() > 1 && (aa.isTop() || bb.isTop()))
    {
        setTop(); // Could be both
        return *this;
    }

    if (aa.isBottom() || bb.isBottom())
    {
        setBottom(); // Undefined
        return *this;
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
        case 1:  mZeroes = ~1; mOnes = 1; break; //A > B
        case 3: //!(A < B) -> A >= B -> no useful information
        case 2:  setTop(); break; //TOP
        default: mZeroes = ~0; mOnes = 0; //A < B, !(A > B) -> A <= B
        }

        break;
    case llvm::CmpInst::ICMP_UGE: // unsigned greater or equal
        switch (compare(aa, bb, false))
        {
        case 0:  //A = B
        case 3:  //!(A < B) -> A >= B
        case 1:  mZeroes = ~1; mOnes = 1; break; //A > B
        case -1: mZeroes = ~0; mOnes = 0; break; //A < B
        default:  setTop(); break; //TOP, !(A > B) -> A <= B
        }

        break;
    case llvm::CmpInst::ICMP_ULT: // unsigned less than
        switch (compare(aa, bb, false))
        {
        case -1: mZeroes = ~1; mOnes = 1; break; //A < B
        case 4: //!(A > B) -> A <= B -> no useful information
        case 2:  setTop(); break; //TOP
        default: mZeroes = ~0; mOnes = 0; //A > B, !(A < B) -> A >= B
        }

        break;
    case llvm::CmpInst::ICMP_ULE: // unsigned less or equal
        switch (compare(aa, bb, false))
        {
        case 0: //A = B
        case 4: //!(A > B) -> A <= B
        case -1: mZeroes = ~1; mOnes = 1; break; //A < B
        case 1: mZeroes = ~0; mOnes = 0; break; //A > B
        default:  setTop(); break; //TOP, !(A < B) -> A >= B
        }

        break;
    case llvm::CmpInst::ICMP_SGT: // signed greater than
        switch (compare(aa, bb, true))
        {
        case 1:  mZeroes = ~1; mOnes = 1; break; //A > B
        case 3: //!(A < B) -> A >= B -> no useful information
        case 2:  setTop(); break; //TOP
        default: mZeroes = ~0; mOnes = 0; //A < B, !(A > B) -> A <= B
        }

        break;
    case llvm::CmpInst::ICMP_SGE: // signed greater or equal
        switch (compare(aa, bb, true))
        {
        case 0:  //A = B
        case 3:  //!(A < B) -> A >= B
        case 1:  mZeroes = ~1; mOnes = 1; break; //A > B
        case -1: mZeroes = ~0; mOnes = 0; break; //A < B
        default:  setTop(); break; //TOP, !(A > B) -> A <= B
        }

        break;
    case llvm::CmpInst::ICMP_SLT: // signed less than
        switch (compare(aa, bb, true))
        {
        case -1: mZeroes = ~1; mOnes = 1; break; //A < B
        case 4: //!(A > B) -> A <= B -> no useful information
        case 2:  setTop(); break; //TOP
        default: mZeroes = ~0; mOnes = 0; //A > B, !(A < B) -> A >= B
        }

        break;
    case llvm::CmpInst::ICMP_SLE: // signed less or equal
        switch (compare(aa, bb, true))
        {
        case 0: //A = B
        case 4: //!(A > B) -> A <= B
        case -1: mZeroes = ~1; mOnes = 1; break; //A < B
        case 1: mZeroes = ~0; mOnes = 0; break; //A > B
        default:  setTop(); break; //TOP, !(A < B) -> A >= B
        }

        break;
    default:
        CANAL_DIE();
    }
    return *this;
}

Bitfield &
Bitfield::fcmp(const Domain &a, const Domain &b,
               llvm::CmpInst::Predicate predicate)
{
    const Float::Interval &aa = checkedCast<Float::Interval>(a),
        &bb = checkedCast<Float::Interval>(b);

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

    return *this;
}

Bitfield &
Bitfield::trunc(const Domain &value)
{
    const Bitfield &bitfield = checkedCast<Bitfield>(value);
    mZeroes = APIntUtils::trunc(bitfield.mZeroes, getBitWidth());
    mOnes = APIntUtils::trunc(bitfield.mOnes, getBitWidth());
    return *this;
}

Bitfield &
Bitfield::zext(const Domain &value)
{
    const Bitfield &bitfield = checkedCast<Bitfield>(value);
    mZeroes = APIntUtils::zext(bitfield.mZeroes, getBitWidth());
    mOnes = APIntUtils::zext(bitfield.mOnes, getBitWidth());

    for (unsigned i = bitfield.mZeroes.getBitWidth(); i < getBitWidth(); ++i)
        APIntUtils::setBit(mZeroes, i);

    return *this;
}

Bitfield &
Bitfield::sext(const Domain &value)
{
    const Bitfield &bitfield = checkedCast<Bitfield>(value);
    mZeroes = APIntUtils::sext(bitfield.mZeroes, getBitWidth());
    mOnes = APIntUtils::sext(bitfield.mOnes, getBitWidth());
    return *this;
}

Bitfield &
Bitfield::fptoui(const Domain &value)
{
    setTop();
    return *this;
}

Bitfield &
Bitfield::fptosi(const Domain &value)
{
    setTop();
    return *this;
}

const llvm::IntegerType &
Bitfield::getValueType() const
{
    return *llvm::Type::getIntNTy(mEnvironment.getContext(), getBitWidth());
}

} // namespace Integer
} // namespace Canal
