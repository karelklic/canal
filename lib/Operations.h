#ifndef LIBCANAL_OPERATIONS_H
#define LIBCANAL_OPERATIONS_H

#include "Domain.h"
#include <map>
#include <vector>

namespace Canal {

class State;
class Machine;
class Domain;
class Environment;
class Constructors;
class OperationsCallback;

/// Context-sensitive flow-insensitive operational abstract
/// interpreter.  Interprets instructions in abstract domain.
///
/// This is an abstract class, which is used as a base class for
/// actual abstract interpretation implementations.
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

    virtual ~Operations() {}

    const Environment &getEnvironment() const { return mEnvironment; }

    /// Interprets current instruction.
    void interpretInstruction(const llvm::Instruction &instruction,
                              State &state);

protected: // Helper functions.
    /// Given a place in source code, return the corresponding variable
    /// from the abstract interpreter state. If the place contains a
    /// constant, fill the provided constant variable with it.
    /// @return
    ///  Returns a pointer to the variable if it is found in the state.
    ///  Returns a pointer to the provided constant if the place contains a
    ///  constant.  Otherwise, it returns NULL.
    const Domain *variableOrConstant(const llvm::Value &place,
                                     State &state,
                                     const llvm::Instruction &instruction,
                                     llvm::OwningPtr<Domain> &constant) const;

    template<typename T> void interpretCall(const T &instruction,
                                            State &state);

    void binaryOperation(const llvm::BinaryOperator &instruction,
                         State &state,
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
    template<typename T> bool getElementPtrOffsets(
        std::vector<Domain*> &result,
        T iteratorStart,
        T iteratorEnd,
        const llvm::Value &place,
        const State &state);

    void castOperation(const llvm::CastInst &instruction,
                       State &state,
                       Domain::CastOperation operation);

    void cmpOperation(const llvm::CmpInst &instruction,
                      State &state,
                      Domain::CmpOperation operation);

protected:
    // Below are methods implementing abstract machine, one method for
    // every LLVM instruction.  The order is based on "LLVM Language
    // Reference Manual", obtained 2012-03-13 from
    // http://llvm.org/docs/LangRef.html.

    /// Return control flow (and optionally a value) from a function
    /// back to the caller.  It's a terminator instruction.
    virtual void ret(const llvm::ReturnInst &instruction,
                     State &state);

    /// Transfer to a different basic block in the current function.
    /// It's a terminator instruction.
    virtual void br(const llvm::BranchInst &instruction,
                    State &state);

    /// Transfer control flow to one of several different places. It is
    /// a generalization of the 'br' instruction, allowing a branch to
    /// occur to one of many possible destinations.  It's a terminator
    /// instruction.
    virtual void switch_(const llvm::SwitchInst &instruction,
                         State &state);

    /// An indirect branch to a label within the current function,
    /// whose address is specified by "address".  It's a terminator
    /// instruction.
    virtual void indirectbr(const llvm::IndirectBrInst &instruction,
                            State &state);

    /// Transfer to a specified function, with the possibility of
    /// control flow transfer to either the 'normal' label or the
    /// 'exception' label.  It's a terminator instruction.
    virtual void invoke(const llvm::InvokeInst &instruction,
                        State &state);

    /// No defined semantics. This instruction is used to inform the
    /// optimizer that a particular portion of the code is not
    /// reachable.  It's a terminator instruction.
    virtual void unreachable(const llvm::UnreachableInst &instruction,
                             State &state);

    /// Sum of two operands.  It's a binary operator.
    virtual void add(const llvm::BinaryOperator &instruction,
                     State &state);

    /// Sum of two operands.  It's a binary operator.  The operands are
    /// floating point or vector of floating point values.
    virtual void fadd(const llvm::BinaryOperator &instruction,
                      State &state);

    /// Difference of two operands.    It's a binary operator.
    virtual void sub(const llvm::BinaryOperator &instruction,
                     State &state);

    /// Difference of two operands.  It's a binary operator.  The
    /// operands are floating point or vector of floating point values.
    virtual void fsub(const llvm::BinaryOperator &instruction,
                      State &state);

    /// Product of two operands.  It's a binary operator.
    virtual void mul(const llvm::BinaryOperator &instruction,
                     State &state);

    /// Product of two operands.  It's a binary operator.
    virtual void fmul(const llvm::BinaryOperator &instruction,
                      State &state);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are integer or vector of integer values.
    virtual void udiv(const llvm::BinaryOperator &instruction,
                      State &state);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are integer or vector of integer values.
    virtual void sdiv(const llvm::BinaryOperator &instruction,
                      State &state);

    /// Quotient of two operands.  It's a binary operator.  The
    /// operands are floating point or vector of floating point values.
    virtual void fdiv(const llvm::BinaryOperator &instruction,
                      State &state);

    /// Unsigned division remainder.  It's a binary operator.
    virtual void urem(const llvm::BinaryOperator &instruction,
                      State &state);

    /// Signed division remainder.  It's a binary operator.
    virtual void srem(const llvm::BinaryOperator &instruction,
                      State &state);

    /// Floating point remainder.  It's a binary operator.
    virtual void frem(const llvm::BinaryOperator &instruction,
                      State &state);

    /// It's a bitwise binary operator.
    virtual void shl(const llvm::BinaryOperator &instruction,
                     State &state);

    /// It's a bitwise binary operator.
    virtual void lshr(const llvm::BinaryOperator &instruction,
                      State &state);

    /// It's a bitwise binary operator.
    virtual void ashr(const llvm::BinaryOperator &instruction,
                      State &state);

    /// It's a bitwise binary operator.
    virtual void and_(const llvm::BinaryOperator &instruction,
                      State &state);

    /// It's a bitwise binary operator.
    virtual void or_(const llvm::BinaryOperator &instruction,
                     State &state);

    /// It's a bitwise binary operator.
    virtual void xor_(const llvm::BinaryOperator &instruction,
                      State &state);

    /// It's a vector operation.
    virtual void extractelement(const llvm::ExtractElementInst &instruction,
                                State &state);

    /// It's a vector operation.
    virtual void insertelement(const llvm::InsertElementInst &instruction,
                               State &state);

    /// It's a vector operation.
    virtual void shufflevector(const llvm::ShuffleVectorInst &instruction,
                               State &state);

    /// It's an aggregate operation.
    virtual void extractvalue(const llvm::ExtractValueInst &instruction,
                              State &state);

    /// It's an aggregate operation.
    virtual void insertvalue(const llvm::InsertValueInst &instruction,
                             State &state);

    /// It's a memory access operation.
    virtual void alloca_(const llvm::AllocaInst &instruction,
                         State &state);

    /// It's a memory access operation.
    virtual void load(const llvm::LoadInst &instruction,
                      State &state);

    /// It's a memory access operation.
    virtual void store(const llvm::StoreInst &instruction,
                       State &state);

    /// It's a memory addressing operation.
    virtual void getelementptr(const llvm::GetElementPtrInst &instruction,
                               State &state);

    /// It's a conversion operation.
    virtual void trunc(const llvm::TruncInst &instruction,
                       State &state);

    /// It's a conversion operation.
    virtual void zext(const llvm::ZExtInst &instruction,
                      State &state);

    /// It's a conversion operation.
    virtual void sext(const llvm::SExtInst &instruction,
                      State &state);

    /// It's a conversion operation.
    virtual void fptrunc(const llvm::FPTruncInst &instruction,
                         State &state);

    /// It's a conversion operation.
    virtual void fpext(const llvm::FPExtInst &instruction,
                       State &state);

    /// It's a conversion operation.
    virtual void fptoui(const llvm::FPToUIInst &instruction,
                        State &state);

    /// It's a conversion operation.
    virtual void fptosi(const llvm::FPToSIInst &instruction,
                        State &state);

    /// It's a conversion operation.
    virtual void uitofp(const llvm::UIToFPInst &instruction,
                        State &state);

    /// It's a conversion operation.
    virtual void sitofp(const llvm::SIToFPInst &instruction,
                        State &state);

    /// It's a conversion operation.
    virtual void ptrtoint(const llvm::PtrToIntInst &instruction,
                          State &state);

    /// It's a conversion operation.
    virtual void inttoptr(const llvm::IntToPtrInst &instruction,
                          State &state);

    /// It's a conversion operation.
    virtual void bitcast(const llvm::BitCastInst &instruction,
                         State &state);

    virtual void icmp(const llvm::ICmpInst &instruction,
                      State &state);

    virtual void fcmp(const llvm::FCmpInst &instruction,
                      State &state);

    virtual void phi(const llvm::PHINode &instruction,
                     State &state);

    virtual void select(const llvm::SelectInst &instruction,
                        State &state);

    virtual void call(const llvm::CallInst &instruction,
                      State &state);

    virtual void va_arg_(const llvm::VAArgInst &instruction,
                        State &state);

#if LLVM_VERSION_MAJOR >= 3
    // Instructions available since LLVM 3.0
    virtual void landingpad(const llvm::LandingPadInst &instruction,
                            State &state);

    /// Resume instruction is available since LLVM 3.0
    /// A terminator instruction that has no successors. Resumes
    /// propagation of an existing (in-flight) exception whose
    /// unwinding was interrupted with a landingpad instruction.
    virtual void resume(const llvm::ResumeInst &instruction,
                        State &state);

    virtual void fence(const llvm::FenceInst &instruction,
                       State &state);

    virtual void cmpxchg(const llvm::AtomicCmpXchgInst &instruction,
                         State &state);

    virtual void atomicrmw(const llvm::AtomicRMWInst &instruction,
                           State &state);
#endif
};

} // namespace Canal

#endif // LIBCANAL_OPERATIONS_H
