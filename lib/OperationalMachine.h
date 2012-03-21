#ifndef CANAL_OPERATIONAL_MACHINE_H
#define CANAL_OPERATIONAL_MACHINE_H

namespace llvm {
    class Instruction;
    class AllocaInst;
    class StoreInst;
    class CallInst;
    class LoadInst;
    class ICmpInst;
    class FCmpInst;
    class GetElementPtrInst;
    class BinaryOperator;
}

namespace Operational {

class State;

/// Instruction interpreter interprets code in abstract domain.
///
/// This is an abstract class, which is used as a base class for
/// actual abstract interpretation implementations.
class Machine
{
public:
    virtual ~Machine() {};

    void interpretInstruction(const llvm::Instruction &instruction, State &state);

protected:
    virtual void alloca_(const llvm::AllocaInst &instruction, State &state);
    virtual void store(const llvm::StoreInst &instruction, State &state);
    virtual void call(const llvm::CallInst &instruction, State &state);
    virtual void load(const llvm::LoadInst &instruction, State &state);
    virtual void icmp(const llvm::ICmpInst &instruction, State &state);
    virtual void fcmp(const llvm::FCmpInst &instruction, State &state);
    virtual void getElementPtr(const llvm::GetElementPtrInst &instruction, State &state);

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
    virtual void shl(const llvm::BinaryOperator &instruction, State &state);
    virtual void lshr(const llvm::BinaryOperator &instruction, State &state);
    virtual void ashr(const llvm::BinaryOperator &instruction, State &state);
    virtual void and_(const llvm::BinaryOperator &instruction, State &state);
    virtual void or_(const llvm::BinaryOperator &instruction, State &state);
    virtual void xor_(const llvm::BinaryOperator &instruction, State &state);
};

} // namespace Operational

#endif
