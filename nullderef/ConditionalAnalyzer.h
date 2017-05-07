#ifndef CONDITION_ANALYZER_H
#define CONDITION_ANALYZER_H 1

#include <queue>
#include <map>

#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>

#include "ErrorCode.h"
#include "PointerStatusMap.h"

using namespace llvm;

class ConditionalAnalyzer {
public:

    ConditionalAnalyzer() { }

    // There are two different cases for an if-statement: 
    // Case 1: One if statement: 2 block, but the first one will branch to the second one
    // Case 2: If and else statement: 2 blocks that branch to the same block. 
    // Everything else whether it's if elseif else or nested statements can be made as a
    // combination of the two cases.
    // A while loop is 2 blocks, where one is checking the condition, while the other one
    // contains the statements of the while loop. 
    ErrorCode analyze(BasicBlock *bb) {
        for (Instruction &I : *bb) {
            // Find the BranchInst to know where to branch to.
            if (BranchInst *bi = dyn_cast<BranchInst>(&I)) {
                
            }
            // Find CmpInst to see if there's another conditional statement inside. 
            if (CmpInst *bi = dyn_cast<CmpInst>(&I)) {
                
            }
        }
        blockQueue.push(bb);
        // When all branches have been visited
        if (completed==true) {
            // Pop the queue into vector. Then delete it.
            // Clear map, since the whole conditional statement is done. 
            return OK;
        }
        return OK;
    }

    // Whenever branching is happening there's either going to be 1 or 2 successors depending
    // if it's a conditional statement or just jumping. 
    void visitBranchInst(BranchInst &I) {
        for(unsigned i = 0; i < I.getNumSuccessors(); i++) {
            BasicBlock *bb = I.getSuccessor(i);
            // Checking whether this Basic Block has been visited. 
            if(visited[bb] != true) {
                // Since a BasicBlock doesn't have a label it has to be added to identify blocks
                bb->setName(std::to_string(visited.size()));
                // Adding it to the map
                visited[bb] = true;
                analyze(bb);
            }
        }
    }

private: 
    std::queue<BasicBlock*> blockQueue; 
    std::map<BasicBlock*,bool> visited;   
    bool completed = false;
};

#endif // CONDITION_ANALYZER_H
