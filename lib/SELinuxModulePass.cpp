#include "Domain.h"
#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <vector>

namespace Canal {

class SELinuxModulePass : public llvm::ModulePass
{
public:
  static char ID;

  SELinuxModulePass() : llvm::ModulePass(ID)
  {
  }

  virtual bool runOnModule(llvm::Module &module)
  {
    const llvm::Function *main = module.getFunction("main");
    if (!main)
      return false;

    const llvm::Function::ArgumentListType &list = main->getArgumentList();

    std::vector<Domain> values;
    interpretFunction(*main, values);
  }

  void interpretFunction(const llvm::Function &F,
			 const std::vector<Domain> &Arguments)
  {
    for (llvm::Function::const_iterator i = F.begin(), e = F.end(); i != e; ++i)
      {
	//interpretBasicBlock(*i, Arguments);
      }
  }

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const
  {
    AU.setPreservesAll();
  }
};

char SELinuxModulePass::ID = 0;

static llvm::RegisterPass<SELinuxModulePass> X("selinux-abstract-interpretation",
					       "SELinux Abstract Interpretation Pass",
					       false /* Only looks at CFG */,
					       false /* Analysis Pass */);

} // namespace Canal
