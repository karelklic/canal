#ifndef LIBCANAL_INTERPRETER_H
#define LIBCANAL_INTERPRETER_H

#include <map>
#include <vector>
#include <llvm/Function.h>

namespace llvm {
class AllocaInst;
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
class Function;
class GetElementPtrInst;
class ICmpInst;
class IndirectBrInst;
class InsertElementInst;
class InsertValueInst;
class Instruction;
class IntToPtrInst;
class InvokeInst;
class LoadInst;
class Module;
class PHINode;
class PtrToIntInst;
class ReturnInst;
class SExtInst;
class SIToFPInst;
class SelectInst;
class ShuffleVectorInst;
class StoreInst;
class SwitchInst;
class TruncInst;
class UIToFPInst;
class UnreachableInst;
class VAArgInst;
class ZExtInst;
#if LLVM_MAJOR >= 3
// Instructions available since LLVM 3.0
class AtomicCmpXchgInst;
class AtomicRMWInst;
class FenceInst;
class LandingPadInst;
class ResumeInst;
#endif
}

namespace Canal {

class State;
class Machine;
class Domain;
class Stack;
class Environment;

/// Context-sensitive flow-insensitive operational abstract
/// interpreter.  Interprets instructions in abstract domain.
///
/// This is an abstract class, which is used as a base class for
/// actual abstract interpretation implementations.
class Interpreter
{
public:
    virtual ~Interpreter() {};

    /// Adds all global variables and constants from a module to the
    /// state.
    void addGlobalVariables(State &state,
                            const Environment &environment);

    /// One step of the interpreter.  Interprets current instruction
    /// and moves to the next one.
    /// @returns
    ///   True if next step is possible.  False on the end of the
    ///   program.
    virtual bool step(Stack &stack,
                      const Environment &environment);

    /// Interprets current instruction.
    void interpretInstruction(Stack &stack,
                              const Environment &environment);

protected:
    // Below are methods implementing abstract machine, one method for
    // every LLVM instruction.  The order is based on "LLVM Language
    // Reference Manual", obtained 2012-03-13 from
    // http://llvm.org/docs/LangRef.html.

    /// Return control flow (and optionally a value) from a function
    /// back to the caller.  It's a terminator instruction.
    virtual void ret(const llvm::ReturnInst &instruction,
                     State &state,
                     const Environment &environment);

    /// Transfer to a different basic block in the current function.
    /// It's a terminator instruction.
    virtual void br(const llvm::BranchInst &instruction,
                    State &state,
                    const Environment &environment);

    /// Transfer control flow to one of several different places. It is
    /// a generalization of the 'br' instruction, allowing a branch to
    /// occur to one of many possible destinations.  It's a terminator
    /// instruction.
    virtual void switch_(const llvm::SwitchInst &instruction,
                         State &state,
                         const Environment &environment);

    /// An indirect branch to a label within the current function,
    /// whose address is specified by "address".  It's a terminator
    /// instruction.
    virtual void indirectbr(const llvm::IndirectBrInst &instruction,
                            State &state,
                            const Environment &environment);

    /// Transfer to a specified function, with the possibility of
    /// control flow transfer to either the 'normal' label or the
    /// 'exception' label.  It's a terminator instruction.
    virtual void invoke(const llvm::InvokeInst &instruction,
                        Stack &stack,
                        const Environment &environment);

    /// No defined semantics. This instruction is used to inform the
    /// optimizer that a particular portion of the code is not
    /// reachable.  It's a terminator instruction.
    virtual void unreachable(const llvm::UnreachableInst &instruction,
                             State &state,
                             const Environment &environment);

    /// Sum of two operands.  It's a binary operator.
    virtual void add(const llvm::BinaryOperator &instruction,
                     State &state,
                     const Environment &environment);

    /// Sum of two operands.  It's a binary operator.  The operands are
    /// floating point or vector of floating point values.
    virtual void fadd(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// Difference of two operands.    It's a binary operator.
    virtual void sub(const llvm::BinaryOperator &instruction,
                     State &state,
                     const Environment &environment);

    /// Difference of two operands.  It's a binary operator.  The
    /// operands are floating point or vector of floating point values.
    virtual void fsub(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// Product of two operands.  It's a binary operator.
    virtual void mul(const llvm::BinaryOperator &instruction,
                     State &state,
                     const Environment &environment);

    /// Product of two operands.  It's a binary operator.
    virtual void fmul(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are integer or vector of integer values.
    virtual void udiv(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are integer or vector of integer values.
    virtual void sdiv(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are floating point or vector of floating point values.
    virtual void fdiv(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// Unsigned division remainder.  It's a binary operator.
    virtual void urem(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// Signed division remainder.  It's a binary operator.
    virtual void srem(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// Floating point remainder.  It's a binary operator.
    virtual void frem(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// It's a bitwise binary operator.
    virtual void shl(const llvm::BinaryOperator &instruction,
                     State &state,
                     const Environment &environment);

    /// It's a bitwise binary operator.
    virtual void lshr(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// It's a bitwise binary operator.
    virtual void ashr(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// It's a bitwise binary operator.
    virtual void and_(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// It's a bitwise binary operator.
    virtual void or_(const llvm::BinaryOperator &instruction,
                     State &state,
                     const Environment &environment);

    /// It's a bitwise binary operator.
    virtual void xor_(const llvm::BinaryOperator &instruction,
                      State &state,
                      const Environment &environment);

    /// It's a vector operation.
    virtual void extractelement(const llvm::ExtractElementInst &instruction,
                                State &state,
                                const Environment &environment);

    /// It's a vector operation.
    virtual void insertelement(const llvm::InsertElementInst &instruction,
                               State &state,
                               const Environment &environment);

    /// It's a vector operation.
    virtual void shufflevector(const llvm::ShuffleVectorInst &instruction,
                               Stack &stack,
                               const Environment &environment);

    /// It's an aggregate operation.
    virtual void extractvalue(const llvm::ExtractValueInst &instruction,
                              State &state,
                              const Environment &environment);

    /// It's an aggregate operation.
    virtual void insertvalue(const llvm::InsertValueInst &instruction,
                             State &state,
                             const Environment &environment);

    /// It's a memory access operation.
    virtual void alloca_(const llvm::AllocaInst &instruction,
                         Stack &stack,
                         const Environment &environment);

    /// It's a memory access operation.
    virtual void load(const llvm::LoadInst &instruction,
                      State &state,
                      const Environment &environment);

    /// It's a memory access operation.
    virtual void store(const llvm::StoreInst &instruction,
                       State &state,
                       const Environment &environment);

    /// It's a memory addressing operation.
    virtual void getelementptr(const llvm::GetElementPtrInst &instruction,
                               Stack &stack,
                               const Environment &environment);

    /// It's a conversion operation.
    virtual void trunc(const llvm::TruncInst &instruction,
                       State &state,
                       const Environment &environment);

    /// It's a conversion operation.
    virtual void zext(const llvm::ZExtInst &instruction,
                      State &state,
                      const Environment &environment);

    /// It's a conversion operation.
    virtual void sext(const llvm::SExtInst &instruction,
                      State &state,
                      const Environment &environment);

    /// It's a conversion operation.
    virtual void fptrunc(const llvm::FPTruncInst &instruction,
                         State &state,
                         const Environment &environment);

    /// It's a conversion operation.
    virtual void fpext(const llvm::FPExtInst &instruction,
                       State &state,
                       const Environment &environment);

    /// It's a conversion operation.
    virtual void fptoui(const llvm::FPToUIInst &instruction,
                        State &state,
                        const Environment &environment);

    /// It's a conversion operation.
    virtual void fptosi(const llvm::FPToSIInst &instruction,
                        State &state,
                        const Environment &environment);

    /// It's a conversion operation.
    virtual void uitofp(const llvm::UIToFPInst &instruction,
                        State &state,
                        const Environment &environment);

    /// It's a conversion operation.
    virtual void sitofp(const llvm::SIToFPInst &instruction,
                        State &state,
                        const Environment &environment);

    /// It's a conversion operation.
    virtual void ptrtoint(const llvm::PtrToIntInst &instruction,
                          State &state,
                          const Environment &environment);

    /// It's a conversion operation.
    virtual void inttoptr(const llvm::IntToPtrInst &instruction,
                          State &state,
                          const Environment &environment);

    /// It's a conversion operation.
    virtual void bitcast(const llvm::BitCastInst &instruction,
                         State &state,
                         const Environment &environment);

    virtual void icmp(const llvm::ICmpInst &instruction,
                      State &state,
                      const Environment &environment);

    virtual void fcmp(const llvm::FCmpInst &instruction,
                      State &state,
                      const Environment &environment);

    virtual void phi(const llvm::PHINode &instruction,
                     State &state,
                     const Environment &environment);

    virtual void select(const llvm::SelectInst &instruction,
                        State &state,
                        const Environment &environment);

    virtual void call(const llvm::CallInst &instruction,
                      Stack &stack,
                      const Environment &environment);

    virtual void va_arg(const llvm::VAArgInst &instruction,
                        State &state,
                        const Environment &environment);

#if LLVM_MAJOR >= 3
    // Instructions available since LLVM 3.0
    virtual void landingpad(const llvm::LandingPadInst &instruction,
                            State &state,
                            const Environment &environment);

    /// Resume instruction is available since LLVM 3.0
    /// A terminator instruction that has no successors. Resumes
    /// propagation of an existing (in-flight) exception whose
    /// unwinding was interrupted with a landingpad instruction.
    virtual void resume(const llvm::ResumeInst &instruction,
                        State &state,
                        const Environment &environment);

    virtual void fence(const llvm::FenceInst &instruction,
                       State &state,
                       const Environment &environment);

    virtual void cmpxchg(const llvm::AtomicCmpXchgInst &instruction,
                         State &state,
                         const Environment &environment);

    virtual void atomicrmw(const llvm::AtomicRMWInst &instruction,
                           State &state,
                           const Environment &environment);
#endif
};

} // namespace Canal

#endif // LIBCANAL_INTERPRETER_H
