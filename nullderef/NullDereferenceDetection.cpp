#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "ErrorCode.h"
#include "PointerTrackerVisitor.h"

#include "Visitor.h"

using namespace llvm;

/*
 * An LLVM pass that statically detects null dereferences.
 *
 * [1] http://llvm.org/docs/ProgrammersManual.html#iterating-over-the-basicblock-in-a-function
 * [2] http://llvm.org/docs/ProgrammersManual.html#turning-an-iterator-into-a-class-pointer-and-vice-versa
 * [3] http://llvm.org/docs/ProgrammersManual.html#the-isa-cast-and-dyn-cast-templates
 * [4] http://llvm.org/docs/WritingAnLLVMPass.html#the-doinitialization-module-method
 * [5] http://llvm.org/docs/WritingAnLLVMPass.html#basic-code-required
 */

namespace {

struct NullDereferenceDetection : public FunctionPass {
    static char ID;
    NullDereferenceDetection() : FunctionPass(ID) {}

    bool runOnFunction(Function &function) override {
        size_t instNumber = 0;
        Visitor visitor;

        errs() << "\n";

        for (BasicBlock &BB : function) { // [1]
            for (Instruction &I : BB) { // [1], little lower
                ErrorCode result;
                try { result = visitor.visit(I); }
                catch (const char *msg) { printError(msg, &I); throw msg; /*for stack trace*/ }

                printResult(result, &I, ++instNumber);
                if ((result & ERROR) == ERROR) break;

                // Dump tracker for every change.
                //errs().changeColor(raw_ostream::BLUE);
                //errs().reverseColor();
                //errs() << ">>> INSTRUCTION " << instNumber << " ";
                //I.dump();
                //errs().resetColor();
                //errs() << "\n";
                //tracker.dump();
                //errs() << "---------------------------------------------------------------------------------------------------------\n\n";
            }
        }

        try {
            errs().changeColor(llvm::raw_ostream::YELLOW);
            errs() << visitor.dump();
            errs().resetColor();
        } catch (const char *msg) { printError(msg); }

        // return true if the function was modified, false otherwise [4]
        return false;
    }

};

}

// Enable the pass for opt [5]
char NullDereferenceDetection::ID = 0;
static RegisterPass<NullDereferenceDetection> X("nullderef", "Null Dereference Check Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);

// Automatically enable the pass for clang.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                                 legacy::PassManagerBase &PM) {
    PM.add(new NullDereferenceDetection());
}

static RegisterStandardPasses
RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
               registerSkeletonPass);
