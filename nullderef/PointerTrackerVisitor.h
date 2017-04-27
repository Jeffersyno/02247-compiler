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
        return OK;
    }

    ErrorCode visitStoreInst(llvm::StoreInst &I) {
        // http://llvm.org/docs/LangRef.html#store-instruction
        Value *op1 = I.getOperand(0); // value to be stored
        Value *op2 = I.getOperand(1); // place to store the value

        // There are 3 different cases we'd like to detect:
        //  - CASE A: op1 is a constant value, either NULL or NON-NULL.
        //    We want to associate this information with op2.
        //  - CASE B: op1 is a non-const value, either it is some non-pointer
        //    variable in the program, or a pointer typed value we already track.
        //  - CASE C: we have status information for the destination (op2). The
        //    destination address might be NULL, in which case we'd want to report
        //    a null dereference.
        //
        // As you can see in our beautiful ASCI diagram below, these 3 cases
        // interleave.
        //
        //   +-+ [const value, unknown dest]
        //   A
        // +-|-+ [non-const value, unknown dest]
        // | |
        // B +-+ [const value, known dest]     +-+
        // |                                     C
        // +---+ [non-const value, known dest] +-+
        //
        // For that reason we split up the handling of this instruction as CASE 1-5.

        // CASE 1: We first detect whether the destination is known to us (CASE C).
        // If we know that it is NIL, then we report an error, regardless of what
        // op1 is (i.e. regardless of CASE A or B).
        if (map.contains(op2) && map.get(op2)->derefIsError()) {
            return handleDerefError(map.get(op2));
        }

        // CASE 2: the value we store is not a pointer type, skip. We only care about
        // the destination address in this case, and we already caught that in CASE 1.
        if (!op1->getType()->isPointerTy()) { return OK; }

        // CASE 3: A constant value is stored in some register (CASE A)(and it is a
        // pointer type (CASE 2), and the destination address is non-null (CASE 1)).
        //    - CASE 3.1: the constant is NULL. Store that op2 points
        //      to a value that stores NIL.
        //    - CASE 3.2: the constant is NOT NULL. Store that op2 points
        //      to a value that stores NON_NIL.
        Constant *c;
        if ((c = dyn_cast<Constant>(op1)) != NULL) {
            if (c->isNullValue()) {
                // We map op2 to a reference status that points to another unmapped PURE status.
                // We use unmapped here because we don't really have a key that should map to it.
                PointerStatus *parent = map.putUnmapped(PointerStatus::createPure(NIL));
                map.put(op2, PointerStatus::createReference(parent));
            }
            else {
                // Similar trick as above.
                PointerStatus *parent = map.putUnmapped(PointerStatus::createPure(NON_NIL));
                map.put(op2, PointerStatus::createReference(parent));
            }
        }
        // CASE 4: A non-constant value is stored in some register, that is,
        // we store a reference to some other object in op2.
        //    - CASE 4.1: We have information about the value stored (e.g. another pointer value).
        //    - CASE 4.2: We don't have information (e.g. reference to some non-pointer value).
        else {
            if (map.contains(op1)) {
                PointerStatus *parent = map.get(op1);
                map.put(op2, PointerStatus::createReference(parent));
            } else {
                PointerStatus *parent = map.putUnmapped(PointerStatus::createPure(NON_NIL));
                map.put(op2, PointerStatus::createReference(parent));
            }
        }

        return OK;
    }

    ErrorCode visitLoadInst(llvm::LoadInst &I) {
        // http://llvm.org/docs/LangRef.html#load-instruction
        Value *op = I.getOperand(0);

        // CASE 1: we have information about the operand being dereferenced. First,
        // we check whether the dereferencing of the operand is an error. If not, we
        // map the result/output of this instruction to the parent of the status
        // information of operand.
        if (map.contains(op)) {
            PointerStatus* status = this->map.get(op);

            if (status->derefIsError()) {
                return handleDerefError(I, status);
            } else if (status->hasParent()) {
                this->map.put(&I, *status->getParent());
            } else {
                // pure value returned here, don't care
            }
        }

        return OK;
    }

    ErrorCode visitGetElementPtrInst(llvm::GetElementPtrInst &I) {
        // http://llvm.org/docs/LangRef.html#getelementptr-instruction

        // Nothing is dereferenced by a GetElementPtr instruction, so
        // no NULL_DEREF here:
        // http://llvm.org/docs/GetElementPtr.html#what-is-dereferenced-by-gep

        Value *op = I.getPointerOperand();
        int64_t offset = -1;
        for (Use *u = I.op_begin()+1; u < I.op_end(); ++u) { // hack
            ConstantInt *c = dyn_cast<ConstantInt>(u->get());
            if (c != NULL && offset == -1) offset = 0;
            if (c != NULL) offset += c->getSExtValue();
        }

        PointerKey key = PointerKey::createOffsetKey(op, offset);
        if (map.contains(key)) {
            map.putAlias(&I, key);
        } else {
            PointerStatus status = map.contains(op)
                    ? *map.get(op)
                    : PointerStatus::createPure(DONT_KNOW);
            map.put(key, status);
            map.putAlias(&I, key);
        }

        return OK;
    }

    ErrorCode visitBitCastInst(BitCastInst &I) {
        //Value *op = I.getOperand(0);

        //// This is a cast, take the same value as the value that is being cast.
        //if (this->map.contains(op)) {
        //    this->map.put(&I, PointerStatus::createAlias(this->map.get(op)));
        //}

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
        //this->map.put(&I, PointerStatus::createPure(DONT_KNOW));
        return OK;
    }

    ErrorCode visitInstruction(llvm::Instruction &I) {
        return OK;
    }

private:

    ErrorCode handleDerefError(PointerStatus *status) {
        PointerStatusValue st = status->getStatus();
        switch (st) {

        // NULL is being dereferenced
        case NIL: return NULL_DEREF;

        // An UNDEFINED value (the result of a NULL dereference) is being dereferenced.
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
