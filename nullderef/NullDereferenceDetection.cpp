#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "PointerTrackerVisitor.h"
#include "TestSuite.h"

using namespace llvm;

/*
 * An LLVM pass that statically detects null dereferences.
 *
 * [1] http://llvm.org/docs/ProgrammersManual.html#iterating-over-the-instruction-in-a-function
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
        PointerTrackerVisitor tracker;

        // TODO implement assignment 2

        errs() << ">> Function " << function.getName() << " <<\n";

        unsigned int instructionIndex = 1;

        try {
            // iterate over all instructions in a function (skip the basic blocks, see [1])
            bool continueLoop = true;
            for (inst_iterator iptr = inst_begin(function), i_end = inst_end(function); iptr != i_end; ++iptr) {

                // get the reference of an instruction from an iterator (see [2])
                Instruction& inst = *iptr;

                switch (tracker.visit(inst)) {
                case OK: break;
                case NULL_DEREF: TestSuite::printResult(NULL_DEREF, inst, instructionIndex); break;
                case MAYBE_NULL_DEREF: TestSuite::printResult(MAYBE_NULL_DEREF, inst, instructionIndex); break;
                case MISSED_DEFINITION: TestSuite::printResult(MISSED_DEFINITION, inst, instructionIndex); continueLoop = false; break;
                case UNKNOWN_ERROR: TestSuite::printResult(UNKNOWN_ERROR, instructionIndex); continueLoop = false; break;
                }
                if(!continueLoop) { break; }

                ++instructionIndex;
            }

            errs() << "This is the dump of the tracker.\n";
            tracker.dump();

        } catch (const char* msg) {
            errs().changeColor(raw_ostream::RED);
            errs() << msg << "\n";
            errs().resetColor();
        }

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
