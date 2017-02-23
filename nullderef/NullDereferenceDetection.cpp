#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
using namespace llvm;

/*
 * An LLVM pass that statically detects null dereferences.
 *
 * [1] http://llvm.org/docs/ProgrammersManual.html#iterating-over-the-instruction-in-a-function
 * [2] http://llvm.org/docs/ProgrammersManual.html#turning-an-iterator-into-a-class-pointer-and-vice-versa
 * [3] http://llvm.org/docs/ProgrammersManual.html#the-isa-cast-and-dyn-cast-templates
 * [4] http://llvm.org/docs/WritingAnLLVMPass.html#the-doinitialization-module-method
 */

namespace {

struct NullDereferenceDetection : public FunctionPass {
    static char ID;
    NullDereferenceDetection() : FunctionPass(ID) {}

    bool runOnFunction(Function &function) override {

        // TODO implement assignment 2

        errs().changeColor(errs().BLUE);
        errs() << ">> Instructions of function " << function.getName() << "<<\n";
        errs().resetColor();

        // iterate over all instructions in a function (skip the basic blocks, see [1])
        for (inst_iterator iptr = inst_begin(function), i_end = inst_end(function); iptr != i_end; ++iptr) {

            // get the reference of an instruction from an iterator (see [2])
            Instruction& inst = *iptr;

            inst.dump();

            // dyn_cast, returns nullptr on failure, which evaluates to false (see [3])
            // works only with pointers (so we need the &)
            if (StoreInst *storeInst = dyn_cast<StoreInst>(&inst))
                dealWithStoreInstruction(*storeInst);
        }

        // return true if the function was modified, false otherwise [4]
        return false;
    }

    void dealWithStoreInstruction(StoreInst &inst) {
        Value *op0 = inst.getOperand(0); // value to be stored
        Value *op1 = inst.getOperand(1); // place to store it

        if (ConstantPointerNull *nil = dyn_cast<ConstantPointerNull>(op0)) {
            for (User *u : op1->users()) {
                if (LoadInst *loadInst = dyn_cast<LoadInst>(u)) {
                    errs().changeColor(errs().RED, true);
                    errs() << "Maybe NullPointerException\n";
                    errs().resetColor();
                    break;
                }
            }
        }
    }
};

}

char NullDereferenceDetection::ID = 0;
static RegisterPass<NullDereferenceDetection> X("nullderef", "Null Dereference Check Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                                 legacy::PassManagerBase &PM) {
    PM.add(new NullDereferenceDetection());
}

static RegisterStandardPasses
RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
               registerSkeletonPass);
