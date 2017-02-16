#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
using namespace llvm;

namespace {

struct NullDereferenceDetection : public FunctionPass {
    static char ID;
    NullDereferenceDetection() : FunctionPass(ID) {}

    bool runOnFunction(Function &function) override {

        // TODO implement assignment 2

        errs() << ">> Instructions of function " << function.getName() << "\n";
        for (auto& basicBlock : function) {
            for (auto&instr : basicBlock) {
                errs() << "Instruction: ";
                instr.dump();
            }
        }

        return false;
    }
};

}

char NullDereferenceDetection::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                                 legacy::PassManagerBase &PM) {
    PM.add(new NullDereferenceDetection());
}

static RegisterStandardPasses
RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
               registerSkeletonPass);
