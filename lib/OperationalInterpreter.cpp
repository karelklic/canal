// Temporary includes
#include <llvm/Constants.h>
#include <llvm/DerivedTypes.h>
#include <llvm/Function.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Analysis/DebugInfo.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/CFG.h>
#include <iostream>


#include "OperationalInterpreter.h"
#include "OperationalState.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Instructions.h>
#include <map>
#include <cassert>
#include <llvm/Support/raw_ostream.h>

namespace Operational {

Interpreter::Interpreter(llvm::Module &module) : mModule(module)
{
}

void Interpreter::interpretFunction(const llvm::Function &function,
                                    State &state,
                                    const std::vector<AbstractValue*> &arguments,
                                    AbstractValue **result)
{
    std::map<const llvm::BasicBlock*, State> blockInputState, blockOutputState;
    llvm::Function::const_iterator itBlock = function.begin(), itBlockEnd = function.end();
    for (; itBlock != itBlockEnd; ++itBlock)
    {
        blockInputState[itBlock] = state;
        blockOutputState[itBlock] = state;
    }

    interpretFunctionBlocks(function.begin(), itBlockEnd, blockInputState, blockOutputState);

    // TODO: fint function return blocks, merge return values, store
    // the result into state.
}

void Interpreter::interpretFunctionBlocks(llvm::Function::const_iterator blockBegin,
                                          llvm::Function::const_iterator blockEnd,
                                          std::map<const llvm::BasicBlock*, State> &blockInputState,
                                          std::map<const llvm::BasicBlock*, State> &blockOutputState)
{
    bool changed = false;
    do {
        llvm::Function::const_iterator itBlock;
        for (itBlock = blockBegin; itBlock != blockEnd; ++itBlock)
        {
            // Merge out states of predecessors to input state of current
            // block.
            llvm::const_pred_iterator itPred = llvm::pred_begin(itBlock),
                itPredEnd = llvm::pred_end(itBlock);
            for (; itPred != itPredEnd; ++itPred)
            {
                assert(&*itBlock != &itBlock->getParent()->getEntryBlock() && "Entry block cannot have predecessors!");
                blockInputState[itBlock].merge(blockOutputState[*itPred]);
            }

            // Interpret all instructions of current block.
            State currentState(blockInputState[itBlock]);
            llvm::BasicBlock::const_iterator itInst = itBlock->begin(),
                itInstEnd = itBlock->end();
            for (; itInst != itInstEnd; ++itInst)
                interpretInstruction(*itInst, currentState);

            // Check if the state changed since the last pass of this
            // block.
            if (currentState != blockOutputState[itBlock])
            {
                changed = true;
                blockOutputState[itBlock] = currentState;
            }
        }
    } while (changed);
}

void Interpreter::interpretInstruction(const llvm::Instruction &instruction, State &state)
{
    if (llvm::isa<llvm::AllocaInst>(instruction))
        alloca_((const llvm::AllocaInst&)instruction, state);
    else if (llvm::isa<llvm::StoreInst>(instruction))
        store((const llvm::StoreInst&)instruction, state);
    else if (llvm::isa<llvm::CallInst>(instruction))
        call((const llvm::CallInst&)instruction, state);
    else if (llvm::isa<llvm::LoadInst>(instruction))
        load((const llvm::LoadInst&)instruction, state);
    else if (llvm::isa<llvm::ICmpInst>(instruction))
        icmp((const llvm::ICmpInst&)instruction, state);
    else if (llvm::isa<llvm::FCmpInst>(instruction))
        fcmp((const llvm::FCmpInst&)instruction, state);
    else if (llvm::isa<llvm::GetElementPtrInst>(instruction))
        getelementptr((const llvm::GetElementPtrInst&)instruction, state);
    else if (llvm::isa<llvm::TerminatorInst>(instruction))
    {
        if (llvm::isa<llvm::ReturnInst>(instruction))
            ret((const llvm::ReturnInst&)instruction, state);
        else if (llvm::isa<llvm::BranchInst>(instruction))
            br((const llvm::BranchInst&)instruction, state);
        else if (llvm::isa<llvm::SwitchInst>(instruction))
            switch_((const llvm::SwitchInst&)instruction, state);
        else if (llvm::isa<llvm::IndirectBrInst>(instruction))
            indirectbr((const llvm::IndirectBrInst&)instruction, state);
        else if (llvm::isa<llvm::InvokeInst>(instruction))
            invoke((const llvm::InvokeInst&)instruction, state);
        else if (llvm::isa<llvm::ResumeInst>(instruction))
            resume((const llvm::ResumeInst&)instruction, state);
        else if (llvm::isa<llvm::UnreachableInst>(instruction))
            unreachable((const llvm::UnreachableInst&)instruction, state);
        else
            llvm::errs() << "Operational::Machine: Unknown terminator instruction: " << instruction << "\n";
    }
    else if (llvm::isa<llvm::BinaryOperator>(instruction))
    {
        const llvm::BinaryOperator &binaryOp = (const llvm::BinaryOperator&)instruction;
        assert(binaryOp.getNumOperands() == 2);
        switch (binaryOp.getOpcode())
	{
	case llvm::Instruction::Add:  add(binaryOp, state);  break;
	case llvm::Instruction::FAdd: fadd(binaryOp, state); break;
	case llvm::Instruction::Sub:  sub(binaryOp, state);  break;
	case llvm::Instruction::FSub: fsub(binaryOp, state); break;
	case llvm::Instruction::Mul:  mul(binaryOp, state);  break;
	case llvm::Instruction::FMul: fmul(binaryOp, state); break;
	case llvm::Instruction::UDiv: udiv(binaryOp, state); break;
	case llvm::Instruction::SDiv: sdiv(binaryOp, state); break;
	case llvm::Instruction::FDiv: fdiv(binaryOp, state); break;
	case llvm::Instruction::URem: urem(binaryOp, state); break;
	case llvm::Instruction::SRem: srem(binaryOp, state); break;
	case llvm::Instruction::FRem: frem(binaryOp, state); break;
	case llvm::Instruction::Shl:  shl(binaryOp, state);  break;
	case llvm::Instruction::LShr: lshr(binaryOp, state); break;
	case llvm::Instruction::AShr: ashr(binaryOp, state); break;
	case llvm::Instruction::And:  and_(binaryOp, state); break;
	case llvm::Instruction::Or:   or_(binaryOp, state);  break;
	case llvm::Instruction::Xor:  xor_(binaryOp, state); break;
	default:
            llvm::errs() << "Operational::Machine: Unknown BinaryOperator instruction: " << binaryOp << "\n";
	}
    }
    else
        llvm::errs() << "Operational::Machine: Unknown instruction: " << instruction << "\n";
}

void Interpreter::ret(const llvm::ReturnInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::br(const llvm::BranchInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::switch_(const llvm::SwitchInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::indirectbr(const llvm::IndirectBrInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::invoke(const llvm::InvokeInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::resume(const llvm::ResumeInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::unreachable(const llvm::UnreachableInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::add(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fadd(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::sub(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fsub(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::mul(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fmul(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::udiv(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::sdiv(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fdiv(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::urem(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::srem(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::frem(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::shl(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::lshr(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::ashr(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::and_(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::or_(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::xor_(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::extractelement(const llvm::ExtractElementInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::insertelement(const llvm::InsertElementInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::shufflevector(const llvm::ShuffleVectorInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::extractvalue(const llvm::ExtractValueInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::insertvalue(const llvm::InsertValueInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::alloca_(const llvm::AllocaInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::load(const llvm::LoadInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::store(const llvm::StoreInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fence(const llvm::FenceInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::cmpxchg(const llvm::AtomicCmpXchgInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::atomicrmw(const llvm::AtomicRMWInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::getelementptr(const llvm::GetElementPtrInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::trunc(const llvm::TruncInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::zext(const llvm::ZExtInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::sext(const llvm::SExtInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fptrunc(const llvm::FPTruncInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fpext(const llvm::FPExtInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fptoui(const llvm::FPToUIInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fptosi(const llvm::FPToSIInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::uitofp(const llvm::UIToFpInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << (const llvm::Instruction&)instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::sitofp(const llvm::SIToFPInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::ptrtoint(const llvm::PtrToIntInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::inttoptr(const llvm::IntToPtrInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::bitcast(const llvm::BitCastInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::icmp(const llvm::ICmpInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::fcmp(const llvm::FCmpInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::phi(const llvm::PHINode &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::select(const llvm::SelectInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::call(const llvm::CallInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::va_arg(const llvm::VAArgInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

void Interpreter::landingpad(const llvm::LandingPadInst &instruction, State &state)
{
    llvm::errs() << "Operational::Interpreter: " << instruction << " is not implemented! Ignoring.\n";
}

} // namespace Operational
