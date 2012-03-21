#include "OperationalMachine.h"
#include "OperationalState.h"
#include <llvm/Instructions.h>
#include <llvm/Support/raw_ostream.h>

namespace Operational {

void Machine::interpretInstruction(const llvm::Instruction &instruction, State &state)
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
        getElementPtr((const llvm::GetElementPtrInst&)instruction, state);
    else if (llvm::isa<llvm::BinaryOperator>(instruction))
    {
        const llvm::BinaryOperator &binaryOp = (const llvm::BinaryOperator&)instruction;
        assert(binaryOp.getNumOperands() == 2);
        switch (binaryOp.getOpcode())
	{
	case llvm::Instruction::Add:  add(binaryOp, state);
	case llvm::Instruction::FAdd: fadd(binaryOp, state);
	case llvm::Instruction::Sub:  sub(binaryOp, state);
	case llvm::Instruction::FSub: fsub(binaryOp, state);
	case llvm::Instruction::Mul:  mul(binaryOp, state);
	case llvm::Instruction::FMul: fmul(binaryOp, state);
	case llvm::Instruction::UDiv: udiv(binaryOp, state);
	case llvm::Instruction::SDiv: sdiv(binaryOp, state);
	case llvm::Instruction::FDiv: fdiv(binaryOp, state);
	case llvm::Instruction::URem: urem(binaryOp, state);
	case llvm::Instruction::SRem: srem(binaryOp, state);
	case llvm::Instruction::FRem: frem(binaryOp, state);
	case llvm::Instruction::Shl:  shl(binaryOp, state);
	case llvm::Instruction::LShr: lshr(binaryOp, state);
	case llvm::Instruction::AShr: ashr(binaryOp, state);
	case llvm::Instruction::And:  and_(binaryOp, state);
	case llvm::Instruction::Or:   or_(binaryOp, state);
	case llvm::Instruction::Xor:  xor_(binaryOp, state);
	default:
            llvm::errs() << "InstructionInterpreter: Unknown BinaryOperator instruction!\n";
	}
    }
    else
        llvm::errs() << "InstructionInterpreter: Unknown instruction!\n";
}

void Machine::alloca_(const llvm::AllocaInst &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: alloca not implemented!\n";
}

void Machine::store(const llvm::StoreInst &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: store not implemented!\n";
}

void Machine::call(const llvm::CallInst &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: call not implemented!\n";
}

void Machine::load(const llvm::LoadInst &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: load not implemented!\n";
}

void Machine::icmp(const llvm::ICmpInst &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: icmp not implemented!\n";
}

void Machine::fcmp(const llvm::FCmpInst &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: fcmp not implemented!\n";
}

void Machine::getElementPtr(const llvm::GetElementPtrInst &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: getElementPtr not implemented!\n";
}

void Machine::add(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: add not implemented!\n";
}

void Machine::fadd(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: fadd not implemented!\n";
}

void Machine::sub(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: sub not implemented!\n";
}

void Machine::fsub(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: fsub not implemented!\n";
}

void Machine::mul(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: mul not implemented!\n";
}

void Machine::fmul(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: fmul not implemented!\n";
}

void Machine::udiv(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: udiv not implemented!\n";
}

void Machine::sdiv(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: sdiv not implemented!\n";
}

void Machine::fdiv(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: fdiv not implemented!\n";
}

void Machine::urem(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: urem not implemented!\n";
}

void Machine::srem(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: srem not implemented!\n";
}

void Machine::frem(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: frem not implemented!\n";
}

void Machine::shl(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: shl not implemented!\n";
}

void Machine::lshr(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: lshr not implemented!\n";
}

void Machine::ashr(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: ashr not implemented!\n";
}

void Machine::and_(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: and not implemented!\n";
}

void Machine::or_(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: or not implemented!\n";
}

void Machine::xor_(const llvm::BinaryOperator &instruction, State &state)
{
    llvm::errs() << "InstructionInterpreter: xor not implemented!\n";
}

} // namespace Operational
