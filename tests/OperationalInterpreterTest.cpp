#include "../lib/OperationalInterpreter.h"
#include "../lib/OperationalState.h"
#include "../lib/AbstractValue.h"
#include "../lib/AbstractPointer.h"
#include <llvm/Value.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/raw_ostream.h>

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

    // Implementation of AbstractValue::operator==().
    virtual bool operator==(const AbstractValue &rhs) const
    {
        // Check if rhs has the same type.
        const SignednessAbstractInteger *integer = dynamic_cast<const SignednessAbstractInteger*>(&rhs);
	if (!integer)
	    return false;

	return mPositive == integer->mPositive &&
	  mNegative == integer->mNegative &&
	  mDivisionByZero == integer->mDivisionByZero;
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

    // Implementation of AbstractValue::printToStream().
    virtual void printToStream(llvm::raw_ostream &o) const
    {
        o << "SignednessAbstractInteger(";
        if (mDivisionByZero)
            o << "division by zero";
        else
        {
            if (mPositive)
                o << "+";
            if (mNegative)
                o << "-";
            if (!mPositive && !mNegative)
                o << "0";
        }
        o << ")";
    }
};

// Abstract machine working on the Sign abstract integer.
class PlusMinusInterpreter : public Operational::Interpreter
{
public:
    PlusMinusInterpreter(llvm::Module &module) : Operational::Interpreter(module)
    {
    }

    void interpretMain()
    {
        // Find main() in the module.
        const llvm::Function &function = *mModule.getFunction("main");

	// Initialize input and output state of all main's basic blocks.
	Operational::State state;
	BlockStateMap blockInputState, blockOutputState;
	llvm::Function::const_iterator itBlock = function.begin(), itBlockEnd = function.end();
	for (; itBlock != itBlockEnd; ++itBlock)
	{
	    blockInputState[itBlock] = state;
	    blockOutputState[itBlock] = state;
	}

	// Interpret the blocks.
	interpretFunctionBlocks(function.begin(), itBlockEnd, blockInputState, blockOutputState);

	// Merge all blocks.
	for (BlockStateMap::const_iterator it = blockOutputState.begin(); it != blockOutputState.end(); ++it)
	{
	    llvm::outs() << it->second;
	    state.merge(it->second);
	}
	llvm::outs() << state;

	llvm::outs() << "Number of variables: " << state.mFunctionVariables.size() << "\n";
	for (Operational::VariablesMap::const_iterator it = state.mFunctionVariables.begin();
	     it != state.mFunctionVariables.end(); ++it)
	{
	    if (it->first->hasName())
	      llvm::outs() << it->first->getName() << " = " << *it->second << "\n";
	    else
	      llvm::outs() << it->first << " = " << *it->second << "\n";
	}
    }

    // Implementation of Operational::Interpreter::alloca().
    virtual void alloca_(const llvm::AllocaInst &instruction, Operational::State &state)
    {
        llvm::Type *type = instruction.getAllocatedType();
        AbstractValue *value = NULL;
        if (type->isIntegerTy())
            value = new SignednessAbstractInteger();
        else if (type->isPointerTy())
            value = new AbstractPointer();
        else
            llvm::errs() << "PlusMinusMachine::alloca: unsupported type: " << instruction << "\n";

        state.mFunctionBlocks.push_back(value);
        AbstractPointer *pointer = new AbstractPointer();
        pointer->mTargets.insert(value);
        llvm::outs() << "Adding " << instruction << ", name: " << instruction.getName() << " " << instruction.hasName() << "\n";
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, AbstractPointer*>(&instruction, pointer));
    }

    // Implementation of Operational::Interpreter::store().
    virtual void store(const llvm::StoreInst &instruction, Operational::State &state)
    {
    }

    // Implementation of Operational::Interpreter::call().
    virtual void call(const llvm::CallInst &instruction, Operational::State &state)
    {
    }

    // Implementation of Operational::Interpreter::load().
    virtual void load(const llvm::LoadInst &instruction, Operational::State &state)
    {
    }

    // Implementation of Operational::Interpreter::add().
    virtual void add(const llvm::BinaryOperator &instruction, Operational::State &state)
    {
        // TODO
        llvm::Value *a = instruction.getOperand(0);
        llvm::Value *b = instruction.getOperand(1);
        if (a->hasName())
        {
            if (b->hasName());
        }

        SignednessAbstractInteger *value = new SignednessAbstractInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, AbstractValue*>(&instruction, value));
    }

    // Implementation of Operational::Interpreter::sub().
    virtual void sub(const llvm::BinaryOperator &instruction, Operational::State &state)
    {
        SignednessAbstractInteger *value = new SignednessAbstractInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, AbstractValue*>(&instruction, value));
    }

    // Implementation of Operational::Interpreter::mul().
    virtual void mul(const llvm::BinaryOperator &instruction, Operational::State &state)
    {
        SignednessAbstractInteger *value = new SignednessAbstractInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, AbstractValue*>(&instruction, value));
    }

    // Implementation of Operational::Interpreter::udiv().
    virtual void udiv(const llvm::BinaryOperator &instruction, Operational::State &state)
    {
        SignednessAbstractInteger *value = new SignednessAbstractInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, AbstractValue*>(&instruction, value));
    }

    // Implementation of Operational::Interpreter::sdiv().
    virtual void sdiv(const llvm::BinaryOperator &instruction, Operational::State &state)
    {
        SignednessAbstractInteger *value = new SignednessAbstractInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, AbstractValue*>(&instruction, value));
    }

    // Implementation of Operational::Interpreter::ret().
    virtual void ret(const llvm::ReturnInst &instruction, Operational::State &state)
    {
        // Do nothing.
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

    PlusMinusInterpreter interpreter(*module);
    interpreter.interpretMain();

    return 0;
}
