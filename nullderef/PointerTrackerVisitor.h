#ifndef POINTER_TRACKER_VISITOR_H
#define POINTER_TRACKER_VISITOR_H

#include <functional>

#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstVisitor.h>

#include "PointerStatusMap.h"

using namespace llvm;

enum VisitResult {
    OK,
    NULL_DEREF,
    MAYBE_NULL_DEREF,
    MISSED_DEFINITION,
    UNKNOWN_ERROR
};

/// Instruction visitor definition
class PointerTrackerVisitor : public llvm::InstVisitor<PointerTrackerVisitor, VisitResult> {
public:
    void dump() {
        this->map.dump();
    }

    VisitResult visitAllocaInst(llvm::AllocaInst &I) {
        if (I.getType()->isPointerTy()) {
            // create a new reference to something we don't know yet
            this->map.put(PointerKey::createLlvmKey(&I), PointerStatus::createReference(NULL));
        }
        return OK;
    }

    VisitResult visitStoreInst(llvm::StoreInst &I) {
        // http://llvm.org/docs/LangRef.html#store-instruction
        Value *op1 = I.getOperand(0); // value to be stored
        Value *op2 = I.getOperand(1); // place to store the value

        // Safety check: in order to store a value we should have detected an allocation first
        if (!this->map.contains(op2)) { return MISSED_DEFINITION; }

        // The address to store op1 is zero: null pointer dereference happening
        if (this->map.get(op2)->isNullDeref()) { return NULL_DEREF; }

        if (dyn_cast<ConstantPointerNull>(op1)) {
            // Constant NULL is stored (must be in a pointer type): we now know that op2 points to a NULL value
            auto parent = this->map.put(&I, PointerStatus::createPure(NIL));
            this->map.get(op2)->setParent(parent);
        } else if (this->map.contains(op1)) {
            // Value is loaded from some other register, and we know it
            this->map.get(op2)->setParent(this->map.get(op1));
        } else {
            // Update the value of the already registered operand
            this->map.put(op2, PointerStatus::createPure(DONT_KNOW));
        }

        return OK;
    }

    VisitResult visitLoadInst(llvm::LoadInst &I) {
        // http://llvm.org/docs/LangRef.html#load-instruction
        Value *op = I.getOperand(0);

        // If the value we're loading is in our map, then consider
        // the same pointer status for the new value.
        if (this->map.contains(op)) {
            PointerStatus* status = this->map.get(op);

            if (status->isNullDeref()) {
                return NULL_DEREF;
            } else if (status->hasParent()) {
                errs() << status->getParent() << "\n";
                this->map.put(&I, *status->getParent());
            }
        } else {
            return MISSED_DEFINITION;
        }
        return OK;
    }

    VisitResult visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
        // http://llvm.org/docs/LangRef.html#getelementptr-instruction
        Value *op = I.getOperand(0);

        //if (this->contains(op)) {
        //    // TODO track information about fetched struct field
        //    if (this->get(op).decr().isNullDeref()) {
        //        return NULL_DEREF;
        //    }
        //}
        return OK;
    }

    VisitResult visitInstruction(llvm::Instruction &I) {
        return OK;
    }

private:
    PointerStatusMap map;
};

#endif // POINTER_TRACKER_VISITOR_H
