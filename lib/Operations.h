#ifndef LIBCANAL_OPERATIONS_H
#define LIBCANAL_OPERATIONS_H

#include "Domain.h"
#include <map>
#include <vector>

namespace Canal {

class Operations
{
protected:
    const Environment &mEnvironment;
    const Constructors &mConstructors;
    OperationsCallback &mCallback;

public:
    Operations(const Environment &environment,
               const Constructors &constructors,
               OperationsCallback &callback);

    const Environment &getEnvironment() const
    {
        return mEnvironment;
    }

    /// Interprets current instruction.
    void interpretInstruction(const llvm::Instruction &instruction,
                              Memory::State &state);

protected: // Helper functions.
    /// Given a place in source code, return the corresponding variable
    /// from the abstract interpreter state. If the place contains a
    /// constant, fill the provided constant variable with it.
    /// @return
    ///  Returns a pointer to the variable if it is found in the state.
    ///  Returns a pointer to the provided constant if the place contains a
    ///  constant.  Otherwise, it returns NULL.
    const Domain *variableOrConstant(const llvm::Value &place,
                                     Memory::State &state,
                                     llvm::OwningPtr<Domain> &constant) const;

    template<typename T> void interpretCall(const T &instruction,
                                            Memory::State &state);

    void binaryOperation(const llvm::BinaryOperator &instruction,
                         Memory::State &state,
                         Domain::BinaryOperation operation);

    // Go through the getelementptr offsets and assembly a list of
    // abstract values representing these offsets.
    // @returns
    //   True if the the operation has been successful.  This means that
    //   state contained all abstract values used as an offset.  False
    //   otherwise.  If false is returned, the result vector is empty.
    // @param result
    //   Vector where all offsets are going to be stored as abstract
    //   values.  Caller takes ownership of the values.
    bool getElementPtrOffsets(
        std::vector<const Domain*> &result,
        llvm::GetElementPtrInst::const_op_iterator iteratorStart,
        llvm::GetElementPtrInst::const_op_iterator iteratorEnd,
        const llvm::Value &place,
        const Memory::State &state);

    void castOperation(const llvm::CastInst &instruction,
                       Memory::State &state,
                       Domain::CastOperation operation);

    void cmpOperation(const llvm::CmpInst &instruction,
                      Memory::State &state,
                      Domain::CmpOperation operation);

protected:
    // Below are methods implementing abstract machine, one method for
    // every LLVM instruction.  The order is based on "LLVM Language
    // Reference Manual", obtained 2012-03-13 from
    // http://llvm.org/docs/LangRef.html.

    /// Return control flow (and optionally a value) from a function
    /// back to the caller.  It's a terminator instruction.
    void ret(const llvm::ReturnInst &instruction,
             Memory::State &state);

    /// Transfer to a different basic block in the current function.
    /// It's a terminator instruction.
    void br(const llvm::BranchInst &instruction,
            Memory::State &state);

    /// Transfer control flow to one of several different places. It is
    /// a generalization of the 'br' instruction, allowing a branch to
    /// occur to one of many possible destinations.  It's a terminator
    /// instruction.
    void switch_(const llvm::SwitchInst &instruction,
                 Memory::State &state);

    /// An indirect branch to a label within the current function,
    /// whose address is specified by "address".  It's a terminator
    /// instruction.
    void indirectbr(const llvm::IndirectBrInst &instruction,
                    Memory::State &state);

    /// Transfer to a specified function, with the possibility of
    /// control flow transfer to either the 'normal' label or the
    /// 'exception' label.  It's a terminator instruction.
    void invoke(const llvm::InvokeInst &instruction,
                Memory::State &state);

    /// No defined semantics. This instruction is used to inform the
    /// optimizer that a particular portion of the code is not
    /// reachable.  It's a terminator instruction.
    void unreachable(const llvm::UnreachableInst &instruction,
                     Memory::State &state);

    /// Sum of two operands.  It's a binary operator.
    void add(const llvm::BinaryOperator &instruction,
             Memory::State &state);

    /// Sum of two operands.  It's a binary operator.  The operands are
    /// floating point or vector of floating point values.
    void fadd(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// Difference of two operands.    It's a binary operator.
    void sub(const llvm::BinaryOperator &instruction,
             Memory::State &state);

    /// Difference of two operands.  It's a binary operator.  The
    /// operands are floating point or vector of floating point values.
    void fsub(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// Product of two operands.  It's a binary operator.
    void mul(const llvm::BinaryOperator &instruction,
             Memory::State &state);

    /// Product of two operands.  It's a binary operator.
    void fmul(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are integer or vector of integer values.
    void udiv(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are integer or vector of integer values.
    void sdiv(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are floating point or vector of floating point values.
    void fdiv(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// Unsigned division remainder.  It's a binary operator.
    void urem(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// Signed division remainder.  It's a binary operator.
    void srem(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// Floating point remainder.  It's a binary operator.
    void frem(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// It's a bitwise binary operator.
    void shl(const llvm::BinaryOperator &instruction,
             Memory::State &state);

    /// It's a bitwise binary operator.
    void lshr(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// It's a bitwise binary operator.
    void ashr(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// It's a bitwise binary operator.
    void and_(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// It's a bitwise binary operator.
    void or_(const llvm::BinaryOperator &instruction,
             Memory::State &state);

    /// It's a bitwise binary operator.
    void xor_(const llvm::BinaryOperator &instruction,
              Memory::State &state);

    /// It's a vector operation.
    void extractelement(const llvm::ExtractElementInst &instruction,
                        Memory::State &state);

    /// It's a vector operation.
    void insertelement(const llvm::InsertElementInst &instruction,
                       Memory::State &state);

    /// It's a vector operation.
    void shufflevector(const llvm::ShuffleVectorInst &instruction,
                       Memory::State &state);

    /// It's an aggregate operation.
    void extractvalue(const llvm::ExtractValueInst &instruction,
                      Memory::State &state);

    /// It's an aggregate operation.
    void insertvalue(const llvm::InsertValueInst &instruction,
                     Memory::State &state);

    /// It's a memory access operation.
    void alloca_(const llvm::AllocaInst &instruction,
                 Memory::State &state);

    /// It's a memory access operation.
    void load(const llvm::LoadInst &instruction,
              Memory::State &state);

    /// It's a memory access operation.
    void store(const llvm::StoreInst &instruction,
               Memory::State &state);

    /// It's a memory addressing operation.
    void getelementptr(const llvm::GetElementPtrInst &instruction,
                       Memory::State &state);

    /// It's a conversion operation.
    void trunc(const llvm::TruncInst &instruction,
               Memory::State &state);

    /// It's a conversion operation.
    void zext(const llvm::ZExtInst &instruction,
              Memory::State &state);

    /// It's a conversion operation.
    void sext(const llvm::SExtInst &instruction,
              Memory::State &state);

    /// It's a conversion operation.
    void fptrunc(const llvm::FPTruncInst &instruction,
                 Memory::State &state);

    /// It's a conversion operation.
    void fpext(const llvm::FPExtInst &instruction,
               Memory::State &state);

    /// It's a conversion operation.
    void fptoui(const llvm::FPToUIInst &instruction,
                Memory::State &state);

    /// It's a conversion operation.
    void fptosi(const llvm::FPToSIInst &instruction,
                Memory::State &state);

    /// It's a conversion operation.
    void uitofp(const llvm::UIToFPInst &instruction,
                Memory::State &state);

    /// It's a conversion operation.
    void sitofp(const llvm::SIToFPInst &instruction,
                Memory::State &state);

    /// It's a conversion operation.
    void ptrtoint(const llvm::PtrToIntInst &instruction,
                  Memory::State &state);

    /// It's a conversion operation.
    void inttoptr(const llvm::IntToPtrInst &instruction,
                  Memory::State &state);

    /// It's a conversion operation.
    void bitcast(const llvm::BitCastInst &instruction,
                 Memory::State &state);

    void icmp(const llvm::ICmpInst &instruction,
              Memory::State &state);

    void fcmp(const llvm::FCmpInst &instruction,
              Memory::State &state);

    void phi(const llvm::PHINode &instruction,
             Memory::State &state);

    void select(const llvm::SelectInst &instruction,
                Memory::State &state);

    void call(const llvm::CallInst &instruction,
              Memory::State &state);

    void va_arg_(const llvm::VAArgInst &instruction,
                 Memory::State &state);

#if LLVM_VERSION_MAJOR >= 3
    // Instructions available since LLVM 3.0
    void landingpad(const llvm::LandingPadInst &instruction,
                    Memory::State &state);

    /// Resume instruction is available since LLVM 3.0
    /// A terminator instruction that has no successors. Resumes
    /// propagation of an existing (in-flight) exception whose
    /// unwinding was interrupted with a landingpad instruction.
    void resume(const llvm::ResumeInst &instruction,
                Memory::State &state);

    void fence(const llvm::FenceInst &instruction,
               Memory::State &state);

    void cmpxchg(const llvm::AtomicCmpXchgInst &instruction,
                 Memory::State &state);

    void atomicrmw(const llvm::AtomicRMWInst &instruction,
                   Memory::State &state);
#endif
};

} // namespace Canal

#endif // LIBCANAL_OPERATIONS_H
