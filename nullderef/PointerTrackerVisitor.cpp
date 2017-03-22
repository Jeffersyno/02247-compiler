#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>

#include "PointerTrackerVisitor.h"

using namespace llvm;

PointerStatus PointerStatus::nil(short depth) { return PointerStatus(NIL, depth); }
PointerStatus PointerStatus::nonNil(short depth) { return PointerStatus(NON_NIL, depth); }
PointerStatus PointerStatus::dontKnow() { return PointerStatus(DONT_KNOW, 0); }
PointerStatus PointerStatus::incr() { return PointerStatus(this->id, this->depth+1); }
PointerStatus PointerStatus::decr() { return PointerStatus(this->id, this->depth-1); }
bool PointerStatus::isNullDeref() { return this->id==NIL && this->depth==0; }
PointerStatus::PointerStatus(short id, short depth): id(id), depth(depth) {}

VisitResult PointerTrackerVisitor::visitAllocaInst(AllocaInst &I) {
    if (I.getType()->isPointerTy()) {

    }
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
    if (this->contains(op)) {
    //    PointerStatus status = this->get(op).decr();
    //    this->update(&I, status);

    //    if (status.isNullDeref()) {
    //        return NULL_DEREF;
    //    }
    }

    return OK;
}

VisitResult PointerTrackerVisitor::visitGetElementPtrInst(GetElementPtrInst &I)
{
    Value *op = I.getOperand(0);

    if (this->contains(op)) {
    //    // TODO track information about fetched struct field
    //    if (this->get(op).decr().isNullDeref()) {
    //        return NULL_DEREF;
    //    }
    }
    return OK;
}

VisitResult PointerTrackerVisitor::visitInstruction(Instruction &I) {
    return OK;
}

void PointerTrackerVisitor::dumpMap()
{
    errs() << "TODO pretty print of map\n";
}
