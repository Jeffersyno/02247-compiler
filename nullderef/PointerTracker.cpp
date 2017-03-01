#include <string>

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>

#include "PointerTracker.h"

using namespace llvm;

std::string pointer_status_toString(PointerStatus status) {
    switch (status) {
    case NIL:	   return "NULL";
    case NON_NIL:  return "NOT NULL";
    case DONTKNOW: return "NULL or NOT NULL";
    }
}

//VisitResult PointerTrackerVisitor::visitAllocaInst(AllocaInst &I) {
//    if (I.getAllocatedType()->isPointerTy()) {
//        std::pair<Value*, PointerStatus> pair(&I, UNINIT);
//        this->pointer_status_map.insert(pair);
//    }
//    return OK;
//}

VisitResult PointerTrackerVisitor::visitStoreInst(StoreInst &I) {
    Value *op1 = I.getOperand(0); // value to be stored
    Value *op2 = I.getOperand(1); // place to store the value

    // CASE 1: constant NULL is stored
    if (ConstantPointerNull *nil = dyn_cast<ConstantPointerNull>(op1)) {
        errs() << "NULL STORED IN ";
        op2->dump();
        this->update(op2, NIL);
    }
    // CASE 2: value is loaded from some other register, and we know it!
    else {
        errs() << "OTHER VALUE STORED IN ";
        op1->dump();
        op2->dump();
        if (this->contains(op1)) {
            errs() << "We know about this assigned!\n";
            this->update(op2, this->get(op1));
        }
    }

    return OK;
}

VisitResult PointerTrackerVisitor::visitLoadInst(LoadInst &I)
{
    return OK;
}

VisitResult PointerTrackerVisitor::visitInstruction(Instruction &I)
{
    return OK;
}

bool PointerTrackerVisitor::update(Value *key, PointerStatus status)
{
    bool result = this->pointer_status_map.erase(key);
    std::pair<Value*, PointerStatus> pair = std::make_pair(key, status);
    this->pointer_status_map.insert(pair);
    return result;
}

PointerStatus PointerTrackerVisitor::get(Value *key)
{
    return this->pointer_status_map[key];
}

bool PointerTrackerVisitor::contains(Value *key)
{
    return this->pointer_status_map.count(key) != 0;
}
