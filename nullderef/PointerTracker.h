#include <functional>

#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstVisitor.h>

enum PointerStatus {
    NIL = 1,
    NON_NIL = 2,
    DONTKNOW = NIL | NON_NIL
};

enum VisitResult {
    OK,
    NULL_DEREF,
    MAYBE_NULL_DEREF,
    UNKNOWN_ERROR
};

typedef llvm::ValueMap<llvm::Value*, PointerStatus> PointerStatusMap;

// Instruction visitor defintion
class PointerTrackerVisitor : public llvm::InstVisitor<PointerTrackerVisitor, VisitResult> {
public:
    VisitResult visitStoreInst(llvm::StoreInst &I);
    VisitResult visitInstruction(llvm::Instruction &I);
    VisitResult visitLoadInst(llvm::LoadInst &I);

private:
    PointerStatusMap pointer_status_map;

    bool update(llvm::Value *key, PointerStatus status);
    PointerStatus get(llvm::Value *key);
    bool contains(llvm::Value *key);
};
