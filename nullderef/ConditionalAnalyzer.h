#ifndef CONDITION_ANALYZER_H
#define CONDITION_ANALYZER_H 1

#include <vector>
#include <queue>

#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/ValueMap.h>

#include "ErrorCode.h"
#include "PointerStatusMap.h"
#include "Visitor.h"

using std::queue;
using std::vector;
using namespace llvm;

/// This class is the start of the support for flow control structures.
/// The goal of this file to control the use of the Vistor/Graph data
/// structure.
///
/// ```
///     ...                 // process normally
///     if (x == NULL) {    //
///         ...             // process with the knowledge that x==NULL
///     } else {
///         ...             // process with the knowledge that x!=NULL
///     }
///     ...                 // merge (take union) of the information
///                         // collected in the branches
/// ```
///
/// The difficult part here is to detect whether we are dealing with
/// an 'if' or an 'if..else' construct.
class ConditionalAnalyzer {
    ValueMap<BasicBlock*, bool> visited;

    Visitor visitor;

public:
    ConditionalAnalyzer() { }

    void analyze(Function &F) {
        for (BasicBlock &BB : F) {
            analyze(&BB);
        }
    }

private:
    void analyze(BasicBlock *BB, BasicBlock *elseBB = NULL) {
        if (isVisited(BB)) { return; }
        markVisited(BB);

        BB->dump();

        BasicBlock *trueBB = NULL;
        BasicBlock *falseBB = NULL;
        findBranchingBasicBlocks(BB, &trueBB, &falseBB);

        if (elseBB != NULL && elseBB == trueBB) { errs() << "HELLO"; }

        if (trueBB != NULL && falseBB != NULL) {
            analyze(trueBB, falseBB);
            if (elseBB != NULL && elseBB == trueBB) { errs() << "MERGE HERE"; }
            analyze(falseBB);
            if (elseBB == NULL) { errs() << "IF..ELSE: MERGE HERE"; }
        }
    }

    void findBranchingBasicBlocks(BasicBlock *BB, BasicBlock **trueBB, BasicBlock **falseBB) {
        for (Instruction &I : *BB) {
            BranchInst *inst;
            if ((inst = dyn_cast<BranchInst>(&I)) != NULL) {
                if (inst->isConditional()) {
                    *trueBB = inst->getSuccessor(0);
                    *falseBB = inst->getSuccessor(1);
                } else {
                    *trueBB = inst->getSuccessor(0);
                }
            }
        }
    }

    bool isVisited(BasicBlock *BB)   { return visited.count(BB) == 1; }
    void markVisited(BasicBlock *BB) { visited[BB] = true; }
};

#endif // CONDITION_ANALYZER_H
