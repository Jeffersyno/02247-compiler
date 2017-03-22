#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>

#include "PointerTrackerVisitor.h"

using namespace llvm;

VisitResult PointerTrackerVisitor::visitAllocaInst(AllocaInst &I) {
    if (I.getType()->isPointerTy()) {

    }

    return OK;
}

VisitResult PointerTrackerVisitor::visitStoreInst(StoreInst &I) {
    // http://llvm.org/docs/LangRef.html#store-instruction
    Value *op1 = I.getOperand(0); // value to be stored
    Value *op2 = I.getOperand(1); // place to store the value

    //// CASE 1: constant NULL is stored (must be in a pointer type)
    //// We now know that op2 points to a NULL value.
    //if (dyn_cast<ConstantPointerNull>(op1)) {
    //    this->update(op2, PointerStatus::nil(2));
    //}
    //// CASE 2: value is loaded from some other register, and we know it!
    //else if (this->contains(op1)) {
    //    this->update(op2, this->get(op1).incr());
    //}
    //// CASE 3: we assign a non-null value
    //else {
    //    this->update(op2, PointerStatus::nonNil(1));
    //}

    return OK;
}

VisitResult PointerTrackerVisitor::visitLoadInst(LoadInst &I) {
    // http://llvm.org/docs/LangRef.html#load-instruction
    Value *op = I.getOperand(0);

    // If the value we're loading is in our map, then consider
    // the same pointer status for the new value.
    // if (this->contains(op)) {
    //    PointerStatus status = this->get(op).decr();
    //    this->update(&I, status);

    //    if (status.isNullDeref()) {
    //        return NULL_DEREF;
    //    }
    //}

    return OK;
}

VisitResult PointerTrackerVisitor::visitGetElementPtrInst(GetElementPtrInst &I)
{
    Value *op = I.getOperand(0);

    //if (this->contains(op)) {
    //    // TODO track information about fetched struct field
    //    if (this->get(op).decr().isNullDeref()) {
    //        return NULL_DEREF;
    //    }
    //}
    return OK;
}

VisitResult PointerTrackerVisitor::visitInstruction(Instruction &I) {
    return OK;
}
