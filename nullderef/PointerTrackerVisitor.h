#ifndef POINTER_TRACKER_VISITOR_H
#define POINTER_TRACKER_VISITOR_H

#include <functional>

#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstVisitor.h>

#include "ErrorCode.h"
#include "PointerStatusMap.h"

using namespace llvm;

/// Instruction visitor definition
class PointerTrackerVisitor : public llvm::InstVisitor<PointerTrackerVisitor, ErrorCode> {
public:
    void dump() {
        this->map.dump();
    }

    ErrorCode visitAllocaInst(llvm::AllocaInst &I) {
        // http://llvm.org/docs/LangRef.html#alloca-instruction

        //if (I.getAllocatedType()->isPointerTy()) {

        // create a new reference to something we don't know yet
        this->map.put(PointerKey::createLlvmKey(&I), PointerStatus::createReference(NULL));
        return OK;
    }

    ErrorCode visitStoreInst(llvm::StoreInst &I) {
        // http://llvm.org/docs/LangRef.html#store-instruction
        Value *op1 = I.getOperand(0); // value to be stored
        Value *op2 = I.getOperand(1); // place to store the value

        // Safety check: in order to store a value we should have detected an allocation first
        if (!this->map.contains(op2)) { return MISSED_DEFINITION; }

        PointerStatus* op2status = this->map.get(op2);

        // The address to store op1 is zero: null pointer dereference happening
        if (op2status->derefIsError()) { return handleDerefError(op2status); }

        if (dyn_cast<ConstantPointerNull>(op1)) {
            // Constant NULL is stored (must be in a pointer type): we now know that op2 points to a NULL value
            auto parent = this->map.put(&I, PointerStatus::createPure(NIL));
            op2status->setParent(parent);
        } else if (this->map.contains(op1)) {
            // Value is loaded from some other register, and we know it
            op2status->setParent(this->map.get(op1));
        } else {
            // Update the value of the already registered operand
            this->map.put(op2, PointerStatus::createPure(DONT_KNOW));
        }

        return OK;
    }

    ErrorCode visitLoadInst(llvm::LoadInst &I) {
        // http://llvm.org/docs/LangRef.html#load-instruction
        Value *op = I.getOperand(0);

        // If the value we're loading is in our map, then consider
        // the same pointer status for the new value.
        if (this->map.contains(op)) {
            PointerStatus* status = this->map.get(op);

            if (status->derefIsError()) {
                return handleDerefError(I, status);
            } else if (status->hasParent()) {
                this->map.put(&I, *status->getParent());
            } else {
                this->map.put(&I, PointerStatus::createPure(DONT_KNOW));
            }
            return OK;
        } else {
            return MISSED_DEFINITION;
        }
    }

    ErrorCode visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
        // http://llvm.org/docs/LangRef.html#getelementptr-instruction

        // Nothing is dereferenced by a GetElementPtr instruction, so
        // no NULL_DEREF here:
        // http://llvm.org/docs/GetElementPtr.html#what-is-dereferenced-by-gep

        Value *op1 = I.getPointerOperand();
        ConstantInt *op2 = dyn_cast<ConstantInt>(I.getOperand(1));

        if (!this->map.contains(op1)) { return MISSED_DEFINITION; }

        PointerStatus *op1status = this->map.get(op1);
        size_t offset = (size_t) op2->getZExtValue();
        PointerKey elemPtrKey = PointerKey::createOffsetKey(op1, offset);
        PointerStatus *elemPtrStatus;

        if (!this->map.contains(elemPtrKey)) {
            elemPtrStatus = this->map.put(elemPtrKey, PointerStatus::createPure(op1status->getStatus()));
        } else {
            elemPtrStatus = this->map.get(elemPtrKey);
        }

        this->map.put(&I, PointerStatus::createAlias(elemPtrStatus));

        return OK;
    }

    ErrorCode visitBitCastInst(BitCastInst &I) {
        Value *op = I.getOperand(0);

        // This is a cast, take the same value as the value that is being cast.
        if (this->map.contains(op)) {
            this->map.put(&I, PointerStatus::createAlias(this->map.get(op)));
        }

        return OK;
    }

    ErrorCode visitMemCpyInst(MemCpyInst &I) {
        Value *source = I.getSource();
        Value *dest = I.getDest();
        if (this->map.contains(source) && this->map.get(source)->derefIsError())
            return NULL_DEREF;
        if (this->map.contains(dest) && this->map.get(dest)->derefIsError())
            return NULL_DEREF;
        return OK;
    }

    ErrorCode visitIntToPtrInst(IntToPtrInst &I) {
        // http://llvm.org/docs/LangRef.html#inttoptr-to-instruction
        this->map.put(&I, PointerStatus::createPure(DONT_KNOW));
        return OK;
    }

    ErrorCode visitInstruction(llvm::Instruction &I) {
        return OK;
    }

private:

    ErrorCode handleDerefError(PointerStatus *status) {
        PointerStatusValue st = status->getStatus();
        switch (st) {
        case NIL: return NULL_DEREF;
        case UNDEFINED: return UNDEFINED_DEREF;
        default: return OK;
        }
    }

    ErrorCode handleDerefError(llvm::Instruction &I, PointerStatus *status) {
        this->map.put(&I, PointerStatus::createPure(UNDEFINED));
        return handleDerefError(status);
    }

    PointerStatusMap map;
};

#endif // POINTER_TRACKER_VISITOR_H
