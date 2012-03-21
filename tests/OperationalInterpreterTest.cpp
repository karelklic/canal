#include "../lib/OperationalInterpreter.h"
#include "../lib/OperationalMachine.h"
#include "../lib/OperationalState.h"
#include "../lib/AbstractValue.h"
#include <llvm/Value.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/ManagedStatic.h>

// This test creates simple LLVM module and simple abstract
// machine. Runs the interpreter using the machine on the module and
// check its results.

// Simple abstract domain for signedness of integers.
// @note
//  The remainder operation is intentionally not implemented, as it
//  would require extending the variables to handle "zero or positive"
//  and "zero or negative" states.
class SignednessAbstractInteger : public AbstractValue
{
public:
    // Is number larger than zero.
    bool mPositive;
    // Is number lower than zero.
    bool mNegative;
    // Error.
    bool mDivisionByZero;

public:
    SignednessAbstractInteger() : mPositive(false), mNegative(false), mDivisionByZero(false)
    {
    }

    // Implementation of AbstractValue::clone().
    // Covariant return type -- it really overrides AbstractValue::clone().
    virtual SignednessAbstractInteger *clone() const
    {
        return new SignednessAbstractInteger(*this);
    }

    // Implementation of AbstractValue::merge().
    virtual void merge(const AbstractValue &abstractValue)
    {
        const SignednessAbstractInteger &value = dynamic_cast<const SignednessAbstractInteger&>(abstractValue);
        mPositive = mPositive || value.mPositive;
        mNegative = mNegative || value.mNegative;
        mDivisionByZero = mDivisionByZero || value.mDivisionByZero;
    }

    // Implementation of AbstractValue::memoryUsage().
    virtual size_t memoryUsage() const
    {
        return sizeof(SignednessAbstractInteger);
    }

    // Implementation of AbstractValue::limitmemoryUsage().
    virtual bool limitMemoryUsage(size_t size)
    {
        // Memory usage of this value cannot be lowered.
        return false;
    }

    // Implementation of AbstractValue::accuracy().
    virtual float accuracy() const
    {
        if (mDivisionByZero)
            return 1.0;
        else if (mPositive)
            return mNegative ? 0.0 : 1.0;
        else
            return 1.0;
    }

    // Implementation of AbstractValue::add().
    virtual void add(const AbstractValue &a, const AbstractValue &b)
    {
        const SignednessAbstractInteger &aa = dynamic_cast<const SignednessAbstractInteger&>(a);
        const SignednessAbstractInteger &bb = dynamic_cast<const SignednessAbstractInteger&>(b);

        // (+)  + (+)   = (+)
        // (+)  + (-)   = (+-)
        // (+)  + (+-)  = (+-)
        // (+)  +  0    = (+)
        // (-)  + (+)   = (+-)
        // (-)  + (-)   = (-)
        // (-)  + (+-)  = (+-)
        // (-)  +  0    = (-)
        // (+-) + (+)   = (+-)
        // (+-) + (-)   = (+-)
        // (+-) + (+-)  = (+-)
        // (+-) +  0    = (+-)
        //  0   + (+)   = (+)
        //  0   + (-)   = (-)
        //  0   + (+-)  = (+-)
        //  0   +  0    =  0
        if (aa.mPositive) // aa is positive or positive/negative
        {
            mPositive = true;
            mNegative = aa.mNegative || bb.mNegative;
        }
        else if (aa.mNegative) // aa is negative
        {
            mPositive = bb.mPositive;
            mNegative = true;
        }
        else // aa is zero
        {
            mPositive = bb.mPositive;
            mNegative = bb.mNegative;
        }
    }

    // Implementation of AbstractValue::sub().
    virtual void sub(const AbstractValue &a, const AbstractValue &b)
    {
        const SignednessAbstractInteger &aa = dynamic_cast<const SignednessAbstractInteger&>(a);
        const SignednessAbstractInteger &bb = dynamic_cast<const SignednessAbstractInteger&>(b);

        // (+)  - (+)  = (+-)
        // (+)  - (-)  = (+)
        // (+)  - (+-) = (+-)
        // (+)  -  0   = (+)
        // (-)  - (+)  = (-)
        // (-)  - (-)  = (+-)
        // (-)  - (+-) = (+-)
        // (-)  -  0   = (-)
        // (+-) - (+)  = (+-)
        // (+-) - (-)  = (+-)
        // (+-) - (+-) = (+-)
        // (+-) -  0   = (+-)
        //  0   - (+)  = (-)
        //  0   - (-)  = (+)
        //  0   - (+-) = (+-)
        //  0   -  0   =  0
        if (aa.mPositive) // aa is positive or positive/negative
        {
            mPositive = true;
            mNegative = aa.mNegative || bb.mPositive;
        }
        else if (aa.mNegative) // aa is negative
        {
            mPositive = bb.mNegative;
            mNegative = true;
        }
        else // aa is zero
        {
            mPositive = bb.mNegative;
            mNegative = bb.mPositive;
        }
    }

    // Implementation of AbstractValue::mul().
    virtual void mul(const AbstractValue &a, const AbstractValue &b)
    {
        const SignednessAbstractInteger &aa = dynamic_cast<const SignednessAbstractInteger&>(a);
        const SignednessAbstractInteger &bb = dynamic_cast<const SignednessAbstractInteger&>(b);

        // (+)  * (+)  = (+)
        // (+)  * (-)  = (-)
        // (+)  * (+-) = (+-)
        // (+)  *  0   =  0
        // (-)  * (+)  = (-)
        // (-)  * (-)  = (+)
        // (-)  * (+-) = (+-)
        // (-)  *  0   =  0
        // (+-) * (+)  = (+-)
        // (+-) * (-)  = (+-)
        // (+-) * (+-) = (+-)
        // (+-) *  0   =  0
        //  0   * (+)  =  0
        //  0   * (-)  =  0
        //  0   * (+-) =  0
        //  0   *  0   =  0
        if (aa.mPositive) // aa is positive or positive/negative
        {
            if (aa.mNegative)
                mPositive = mNegative = (bb.mPositive || bb.mNegative);
            else
            {
                mPositive = bb.mPositive;
                mNegative = bb.mNegative;
            }
        }
        else if (aa.mNegative) // aa is negative
        {
            mPositive = bb.mNegative;
            mNegative = bb.mPositive;
        }
        else // aa is zero
            mPositive = mNegative = false;
    }

    // Implementation of AbstractValue::div().
    virtual void div(const AbstractValue &a, const AbstractValue &b)
    {
        const SignednessAbstractInteger &aa = dynamic_cast<const SignednessAbstractInteger&>(a);
        const SignednessAbstractInteger &bb = dynamic_cast<const SignednessAbstractInteger&>(b);

        // (+)  / (+)  = (+)
        // (+)  / (-)  = (-)
        // (+)  / (+-) = (+-)
        // (+)  /  0   = div by zero
        // (-)  / (+)  = (-)
        // (-)  / (-)  = (+)
        // (-)  / (+-) = (+-)
        // (-)  /  0   = div by zero
        // (+-) / (+)  = (+-)
        // (+-) / (-)  = (+-)
        // (+-) / (+-) = (+-)
        // (+-) /  0   = div by zero
        //  0   / (+)  =  0
        //  0   / (-)  =  0
        //  0   / (+-) =  0
        //  0   /  0   = div by zero
        mDivisionByZero = !bb.mNegative & !bb.mPositive;
        if (aa.mPositive) // aa is positive or positive/negative
        {
            mPositive = bb.mPositive | aa.mNegative;
            mNegative = bb.mNegative | aa.mNegative;
        }
        else if (aa.mNegative) // aa is negative
        {
            mPositive = bb.mNegative;
            mNegative = bb.mPositive;
        }
        else // aa is zero
            mPositive = mNegative = false;
    }
};

// Abstract machine working on the Sign abstract integer.
class PlusMinusMachine : public Operational::Machine
{
public:
    // Implementation of Operational::Machine::alloca().
    virtual void alloca_(const llvm::AllocaInst &instruction, State &state)
    {
        Type *type = instruction.getAllocatedType();
        if (type->isIntegerTy())
        {
            state.mFunctionBlocks.append(new SignednessAbstractInteger());
        }
    }

    // Implementation of Operational::Machine::store().
    virtual void store(const llvm::StoreInst &instruction, State &state)
    {
    }

    // Implementation of Operational::Machine::call().
    virtual void call(const llvm::CallInst &instruction, State &state)
    {
    }

    // Implementation of Operational::Machine::load().
    virtual void load(const llvm::LoadInst &instruction, State &state)
    {
    }

    // Implementation of Operational::Machine::add().
    virtual void add(const llvm::BinaryOperator &I, Operational::State &S)
    {
        llvm::Value *a = I.getOperand(0);
        llvm::Value *b = I.getOperand(1);
        if (a->hasName())
        {
            if (b->hasName());
        }
    }

    // Implementation of Operational::Machine::sub().
    virtual void sub(const llvm::BinaryOperator &I, Operational::State &S)
    {
    }

    // Implementation of Operational::Machine::mul().
    virtual void mul(const llvm::BinaryOperator &I, Operational::State &S)
    {
    }

    // Implementation of Operational::Machine::udiv().
    virtual void udiv(const llvm::BinaryOperator &I, Operational::State &S)
    {
    }

    // Implementation of Operational::Machine::sdiv().
    virtual void sdiv(const llvm::BinaryOperator &I, Operational::State &S)
    {
    }
};

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1;

    llvm::LLVMContext &context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj y;  // Call llvm_shutdown() on exit.

    llvm::SMDiagnostic err;
    llvm::Module *module = llvm::ParseIRFile(argv[1], err, context);
    if (!module)
        return 1;

    PlusMinusMachine machine;
    Operational::Interpreter interpreter(machine, *module);

    const llvm::Function *main = module->getFunction("main");
    Operational::State state;
    interpreter.interpretFunction(*main, state);

    return 0;
}
