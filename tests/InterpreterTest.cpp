#include "../lib/Interpreter.h"
#include "../lib/State.h"
#include "../lib/Value.h"
#include "../lib/Pointer.h"
#include "../lib/Utils.h"
#include <llvm/Value.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRReader.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/raw_ostream.h>

namespace Canal {

// This test creates simple LLVM module and simple abstract
// machine. Runs the interpreter using the machine on the module and
// check its results.

// Simple abstract domain for signedness of integers.
// @note
//  The remainder operation is intentionally not implemented, as it
//  would require extending the variables to handle "zero or positive"
//  and "zero or negative" states.
class SignednessInteger : public Value
{
public:
    // Is number larger than zero.
    bool mPositive;
    // Is number lower than zero.
    bool mNegative;
    // Error.
    bool mDivisionByZero;

public:
    SignednessInteger() : mPositive(false), mNegative(false), mDivisionByZero(false)
    {
    }

    // Implementation of Value::clone().
    // Covariant return type -- it really overrides Value::clone().
    virtual SignednessInteger *clone() const
    {
        return new SignednessInteger(*this);
    }

    // Implementation of Value::operator==().
    virtual bool operator==(const Value &rhs) const
    {
        // Check if rhs has the same type.
        const SignednessInteger *integer = dynamic_cast<const SignednessInteger*>(&rhs);
	if (!integer)
	    return false;

	return mPositive == integer->mPositive &&
	  mNegative == integer->mNegative &&
	  mDivisionByZero == integer->mDivisionByZero;
    }

    // Implementation of Value::merge().
    virtual void merge(const Value &abstractValue)
    {
        const SignednessInteger &value = dynamic_cast<const SignednessInteger&>(abstractValue);
        mPositive = mPositive || value.mPositive;
        mNegative = mNegative || value.mNegative;
        mDivisionByZero = mDivisionByZero || value.mDivisionByZero;
    }

    // Implementation of Value::memoryUsage().
    virtual size_t memoryUsage() const
    {
        return sizeof(SignednessInteger);
    }

    // Implementation of Value::limitmemoryUsage().
    virtual bool limitMemoryUsage(size_t size)
    {
        // Memory usage of this value cannot be lowered.
        return false;
    }

    // Implementation of Value::accuracy().
    virtual float accuracy() const
    {
        if (mDivisionByZero)
            return 1.0;
        else if (mPositive)
            return mNegative ? 0.0 : 1.0;
        else
            return 1.0;
    }

    // Implementation of Value::add().
    virtual void add(const Value &a, const Value &b)
    {
        const SignednessInteger &aa = dynamic_cast<const SignednessInteger&>(a);
        const SignednessInteger &bb = dynamic_cast<const SignednessInteger&>(b);

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

    // Implementation of Value::sub().
    virtual void sub(const Value &a, const Value &b)
    {
        const SignednessInteger &aa = dynamic_cast<const SignednessInteger&>(a);
        const SignednessInteger &bb = dynamic_cast<const SignednessInteger&>(b);

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

    // Implementation of Value::mul().
    virtual void mul(const Value &a, const Value &b)
    {
        const SignednessInteger &aa = dynamic_cast<const SignednessInteger&>(a);
        const SignednessInteger &bb = dynamic_cast<const SignednessInteger&>(b);

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

    // Implementation of Value::div().
    virtual void div(const Value &a, const Value &b)
    {
        const SignednessInteger &aa = dynamic_cast<const SignednessInteger&>(a);
        const SignednessInteger &bb = dynamic_cast<const SignednessInteger&>(b);

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

    // Implementation of Value::printToStream().
    virtual void printToStream(llvm::raw_ostream &o) const
    {
        o << "SignednessInteger(";
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

//  machine working on the Sign abstract integer.
class PlusMinusInterpreter : public Interpreter
{
public:
    PlusMinusInterpreter(llvm::Module &module) : Interpreter(module)
    {
    }

    void interpretMain()
    {
        // Find main() in the module.
        const llvm::Function &function = *mModule.getFunction("main");

	// Initialize input and output state of all main's basic blocks.
	State state;
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
	for (VariablesMap::const_iterator it = state.mFunctionVariables.begin();
	     it != state.mFunctionVariables.end(); ++it)
	{
	    if (it->first->hasName())
            {
                const Pointer::InclusionBased *pointer = dynamic_cast<const Pointer::InclusionBased*>(it->second);
                if (pointer && pointer->mTargets.size() == 1)
                {
                    llvm::outs() << it->first->getName() << " = " << *pointer->mTargets.begin()->second.dereference(state) << "\n";
                }
                else
                    llvm::outs() << it->first->getName() << " = " << *it->second << "\n";
            }
	    else
                llvm::outs() << it->first << " = " << *it->second << "\n";
	}
    }

    // Implementation of Interpreter::alloca().
    virtual void alloca_(const llvm::AllocaInst &instruction, State &state)
    {
        // Alloca allocates either a single item, or an array of
        // items.

        if (state.mFunctionVariables.end() != state.mFunctionVariables.find(&instruction))
        {
            // TODO: handle allocation size change
            return;
        }

        llvm::Type *type = instruction.getAllocatedType();
        Value *value = NULL;
        if (type->isIntegerTy())
            value = new SignednessInteger();
        else if (type->isPointerTy())
            value = new Pointer::InclusionBased();
        else
            CANAL_DIE();

        state.mFunctionBlocks.push_back(value);

        Pointer::Target target;
        target.setBlockOffset(state.mFunctionBlocks.size() - 1,
                              Pointer::Target::FunctionBlock);

        Pointer::InclusionBased *pointer = new Pointer::InclusionBased();
        pointer->mTargets[&instruction] = target;

        llvm::outs() << "Adding " << instruction << ", name: " << instruction.getName() << " " << instruction.hasName() << "\n";

        state.mFunctionVariables.insert(std::pair<const llvm::Value*, Pointer::InclusionBased*>(&instruction, pointer));
    }

    // Implementation of Interpreter::store().
    virtual void store(const llvm::StoreInst &instruction, State &state)
    {
    }

    // Implementation of Interpreter::call().
    virtual void call(const llvm::CallInst &instruction, State &state)
    {
    }

    // Implementation of Interpreter::load().
    virtual void load(const llvm::LoadInst &instruction, State &state)
    {
    }

    // Implementation of Interpreter::add().
    virtual void add(const llvm::BinaryOperator &instruction, State &state)
    {
        // TODO
        llvm::Value *a = instruction.getOperand(0);
        llvm::Value *b = instruction.getOperand(1);
        if (a->hasName())
        {
            if (b->hasName());
        }

        SignednessInteger *value = new SignednessInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, Value*>(&instruction, value));
    }

    // Implementation of Interpreter::sub().
    virtual void sub(const llvm::BinaryOperator &instruction, State &state)
    {
        SignednessInteger *value = new SignednessInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, Value*>(&instruction, value));
    }

    // Implementation of Interpreter::mul().
    virtual void mul(const llvm::BinaryOperator &instruction, State &state)
    {
        SignednessInteger *value = new SignednessInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, Value*>(&instruction, value));
    }

    // Implementation of Interpreter::udiv().
    virtual void udiv(const llvm::BinaryOperator &instruction, State &state)
    {
        SignednessInteger *value = new SignednessInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, Value*>(&instruction, value));
    }

    // Implementation of Interpreter::sdiv().
    virtual void sdiv(const llvm::BinaryOperator &instruction, State &state)
    {
        SignednessInteger *value = new SignednessInteger();
        state.mFunctionVariables.insert(std::pair<const llvm::Value*, Value*>(&instruction, value));
    }

    // Implementation of Interpreter::ret().
    virtual void ret(const llvm::ReturnInst &instruction, State &state)
    {
        // Do nothing.
    }
};

} // namespace Canal

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

    Canal::PlusMinusInterpreter interpreter(*module);
    interpreter.interpretMain();

    return 0;
}
