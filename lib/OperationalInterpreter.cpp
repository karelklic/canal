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
#include "OperationalMachine.h"
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <map>
#include <cassert>

namespace Operational {

Interpreter::Interpreter(Machine &machine, llvm::Module &module) : mMachine(machine), mModule(module)
{
}

void Interpreter::interpretFunction(const llvm::Function &func, const State &state)
{
    std::map<const llvm::BasicBlock*, State> InputState, OutputState;

    for (llvm::Function::const_iterator BB = func.begin(), BBE = func.end(); BB != BBE; ++BB)
    {
        InputState[BB] = state;
        OutputState[BB] = state;
    }

    bool changed = false;
    do {
        // BB - Basic block, BBE - End of Basic blocks
        for (llvm::Function::const_iterator BB = func.begin(), BBE = func.end(); BB != BBE; ++BB)
        {
            // Merge out states of predecessors to input state of current
            // block.
            for (llvm::const_pred_iterator PI = llvm::pred_begin(BB), PE = llvm::pred_end(BB); PI != PE; ++PI)
            {
                assert(&*BB != &func.getEntryBlock() && "Entry block cannot have predecessors!");
                InputState[BB].merge(OutputState[*PI]);
            }

            // Interpret all instructions of current block.
            State CurrentState(InputState[BB]);
            llvm::BasicBlock::const_iterator Inst = BB->begin(), InstEnd = BB->end();
            for (; Inst != InstEnd; ++Inst)
                mMachine.interpretInstruction(*Inst, CurrentState);

            // Check if the state changed since the last pass of this
            // block.
            if (CurrentState != OutputState[BB])
            {
                changed = true;
                OutputState[BB] = CurrentState;
            }
        }
    } while (changed);
}

} // namespace Operational
