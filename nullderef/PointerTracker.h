#include <functional>

#include <llvm/IR/Value.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstVisitor.h>

/**
 * Class that stores information about pointers.
 *             {NIL, NON_NIL}
 *               //     \\
 *           {NIL}     {NON_NIL}
 *               \\     //
 *                  {}
**/
class PointerStatus {
    static const short NIL = 1;
    static const short NON_NIL = 2;
    static const short DONT_KNOW = 1 | 2;

    /// Denote in which state a pointer is:
    ///  - NIL:       it points to NULL somewhere down the pointer chain
    ///  - NON_NIL:   it is not a NULL pointer
    ///  - DONT_KNOW: we have no idea
    const short id;

    /// The number of times we need to dereference before we dereference NULL.
    /// EXAMPLE:
    ///   - int *ptr1   = NULL;     depth=1,
    ///   - int **ptr2  = &ptr1;    depth=2,
    ///   - int ***ptr3 = &ptr2;    depth=3, ...
    const short depth;

public:
    static PointerStatus nil(short depth);
    static PointerStatus nonNil(short depth);
    static PointerStatus dontKnow();

    /// Increment the depth of the pointer by 1
    PointerStatus incr();

    /// Decrement the depth of the pointer by 1
    PointerStatus decr();

    /// id=NIL and depth=0
    bool isNullDeref();

    PointerStatus(short id = DONT_KNOW, short depth = 0);
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
    VisitResult visitAllocaInst(llvm::AllocaInst &I);
    VisitResult visitStoreInst(llvm::StoreInst &I);
    VisitResult visitLoadInst(llvm::LoadInst &I);

    VisitResult visitInstruction(llvm::Instruction &I);

    void dumpMap();

private:
    PointerStatusMap pointer_status_map;

    bool update(llvm::Value *key, PointerStatus status);
    PointerStatus get(llvm::Value *key);
    bool contains(llvm::Value *key);
};
