#ifndef POINTER_TRACKER_VISITOR_H
#define POINTER_TRACKER_VISITOR_H

#include <functional>

#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstVisitor.h>

#include "PointerStatusMap.h"

enum VisitResult {
    OK,
    NULL_DEREF,
    MAYBE_NULL_DEREF,
    UNKNOWN_ERROR
};

// Instruction visitor defintion
class PointerTrackerVisitor : public llvm::InstVisitor<PointerTrackerVisitor, VisitResult> {
public:
    VisitResult visitAllocaInst(llvm::AllocaInst &I);
    VisitResult visitStoreInst(llvm::StoreInst &I);
    VisitResult visitLoadInst(llvm::LoadInst &I);
    VisitResult visitGetElementPtrInst(llvm::GetElementPtrInst &I);

    VisitResult visitInstruction(llvm::Instruction &I);
private:
    PointerStatusMap map;
};

#endif // POINTER_TRACKER_VISITOR_H
