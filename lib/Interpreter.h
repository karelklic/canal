#ifndef LIBCANAL_INTERPRETER_H
#define LIBCANAL_INTERPRETER_H

#include <map>
#include <vector>
#include <llvm/Function.h>

namespace llvm {
    class AllocaInst;
    class AtomicCmpXchgInst;
    class AtomicRMWInst;
    class BasicBlock;
    class BinaryOperator;
    class BitCastInst;
    class BranchInst;
    class CallInst;
    class ExtractElementInst;
    class ExtractValueInst;
    class FCmpInst;
    class FPExtInst;
    class FPToSIInst;
    class FPToUIInst;
    class FPTruncInst;
    class FenceInst;
    class Function;
    class GetElementPtrInst;
    class ICmpInst;
    class IndirectBrInst;
    class InsertElementInst;
    class InsertValueInst;
    class Instruction;
    class IntToPtrInst;
    class InvokeInst;
    class LandingPadInst;
    class LoadInst;
    class Module;
    class PHINode;
    class PtrToIntInst;
    class ResumeInst;
    class ReturnInst;
    class SExtInst;
    class SIToFPInst;
    class SelectInst;
    class ShuffleVectorInst;
    class StoreInst;
    class SwitchInst;
    class TruncInst;
    class UIToFpInst;
    class UnreachableInst;
    class VAArgInst;
    class ZExtInst;
}

namespace Canal {

class State;
class Machine;
class Value;
class Stack;

// Context-sensitive operational abstract interpreter.  Interprets
// instructions in abstract domain.
//
/// This is an abstract class, which is used as a base class for
/// actual abstract interpretation implementations.
class Interpreter
{
public:
    virtual ~Interpreter() {};

    // One step of the interpreter.  Interprets current instruction
    // and moves to the next one.
    // @returns
    //   True if next step is possible.  False on the end of the
    //   program.
    virtual bool step(Stack &stack);

    // Interprets current instruction.
    void interpretInstruction(Stack &stack);

protected:
    // Below are methods implementing abstract machine, one method for
    // every LLVM instruction.  The order is based on "LLVM Language
    // Reference Manual", obtained 2012-03-13 from
    // http://llvm.org/docs/LangRef.html.

    // Terminator instructions
    // Return control flow (and optionally a value) from a function back
    // to the caller.
    virtual void ret(const llvm::ReturnInst &instruction, State &state);
    // Transfer to a different basic block in the current function.
    virtual void br(const llvm::BranchInst &instruction, State &state);
    // Transfer control flow to one of several different places. It is
    // a generalization of the 'br' instruction, allowing a branch to
    // occur to one of many possible destinations.
    virtual void switch_(const llvm::SwitchInst &instruction, State &state);
    // An indirect branch to a label within the current function,
    // whose address is specified by "address".
    virtual void indirectbr(const llvm::IndirectBrInst &instruction, State &state);
    // Transfer to a specified function, with the possibility of
    // control flow transfer to either the 'normal' label or the
    // 'exception' label.
    virtual void invoke(const llvm::InvokeInst &instruction, Stack &stack);
    // A terminator instruction that has no successors. Resumes
    // propagation of an existing (in-flight) exception whose
    // unwinding was interrupted with a landingpad instruction.
    virtual void resume(const llvm::ResumeInst &instruction, State &state);
    // No defined semantics. This instruction is used to inform the
    // optimizer that a particular portion of the code is not
    // reachable.
    virtual void unreachable(const llvm::UnreachableInst &instruction, State &state);

    // Binary operators
    // Sum of two operands.
    virtual void add(const llvm::BinaryOperator &instruction, State &state);
    // Sum of two operands. The operands are floating point or vector of
    // floating point values.
    virtual void fadd(const llvm::BinaryOperator &instruction, State &state);
    // Difference of two operands.
    virtual void sub(const llvm::BinaryOperator &instruction, State &state);
    // Difference of two operands.  The operands are floating point or
    // vector of floating point values.
    virtual void fsub(const llvm::BinaryOperator &instruction, State &state);
    // Product of two operands.
    virtual void mul(const llvm::BinaryOperator &instruction, State &state);
    // Product of two operands.
    virtual void fmul(const llvm::BinaryOperator &instruction, State &state);
    // Quotient of two operands. The operands are integer or vector of
    // integer values.
    virtual void udiv(const llvm::BinaryOperator &instruction, State &state);
    // Quotient of two operands. The operands are integer or vector of
    // integer values.
    virtual void sdiv(const llvm::BinaryOperator &instruction, State &state);
    // Quotient of two operands. The operands are floating point or
    // vector of floating point values.
    virtual void fdiv(const llvm::BinaryOperator &instruction, State &state);
    virtual void urem(const llvm::BinaryOperator &instruction, State &state);
    virtual void srem(const llvm::BinaryOperator &instruction, State &state);
    virtual void frem(const llvm::BinaryOperator &instruction, State &state);

    // Bitwise binary operators
    virtual void shl(const llvm::BinaryOperator &instruction, State &state);
    virtual void lshr(const llvm::BinaryOperator &instruction, State &state);
    virtual void ashr(const llvm::BinaryOperator &instruction, State &state);
    virtual void and_(const llvm::BinaryOperator &instruction, State &state);
    virtual void or_(const llvm::BinaryOperator &instruction, State &state);
    virtual void xor_(const llvm::BinaryOperator &instruction, State &state);

    // Vector Operations
    virtual void extractelement(const llvm::ExtractElementInst &instruction, State &state);
    virtual void insertelement(const llvm::InsertElementInst &instruction, State &state);
    virtual void shufflevector(const llvm::ShuffleVectorInst &instruction, State &state);

    // Aggregate Operations
    virtual void extractvalue(const llvm::ExtractValueInst &instruction, State &state);
    virtual void insertvalue(const llvm::InsertValueInst &instruction, State &state);

    // Memory Access and Adressing Operations
    virtual void alloca_(const llvm::AllocaInst &instruction, Stack &stack);
    virtual void load(const llvm::LoadInst &instruction, State &state);
    virtual void store(const llvm::StoreInst &instruction, State &state);
    virtual void fence(const llvm::FenceInst &instruction, State &state);
    virtual void cmpxchg(const llvm::AtomicCmpXchgInst &instruction, State &state);
    virtual void atomicrmw(const llvm::AtomicRMWInst &instruction, State &state);
    virtual void getelementptr(const llvm::GetElementPtrInst &instruction, Stack &stack);

    // Conversion Operations
    virtual void trunc(const llvm::TruncInst &instruction, State &state);
    virtual void zext(const llvm::ZExtInst &instruction, State &state);
    virtual void sext(const llvm::SExtInst &instruction, State &state);
    virtual void fptrunc(const llvm::FPTruncInst &instruction, State &state);
    virtual void fpext(const llvm::FPExtInst &instruction, State &state);
    virtual void fptoui(const llvm::FPToUIInst &instruction, State &state);
    virtual void fptosi(const llvm::FPToSIInst &instruction, State &state);
    virtual void uitofp(const llvm::UIToFpInst &instruction, State &state);
    virtual void sitofp(const llvm::SIToFPInst &instruction, State &state);
    virtual void ptrtoint(const llvm::PtrToIntInst &instruction, State &state);
    virtual void inttoptr(const llvm::IntToPtrInst &instruction, State &state);
    virtual void bitcast(const llvm::BitCastInst &instruction, State &state);

    // Other Operations
    virtual void icmp(const llvm::ICmpInst &instruction, State &state);
    virtual void fcmp(const llvm::FCmpInst &instruction, State &state);
    virtual void phi(const llvm::PHINode &instruction, State &state);
    virtual void select(const llvm::SelectInst &instruction, State &state);
    virtual void call(const llvm::CallInst &instruction, Stack &stack);
    virtual void va_arg(const llvm::VAArgInst &instruction, State &state);
    virtual void landingpad(const llvm::LandingPadInst &instruction, State &state);
};

} // namespace Canal

#endif // LIBCANAL_INTERPRETER_H
