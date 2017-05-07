#ifndef INST_VISITOR_H
#define INST_VISITOR_H 1

#include <llvm/IR/InstVisitor.h>

#include "PointerGraph.h"
#include "ErrorCode.h"
#include "ConditionalAnalyzer.h"

using graph::Graph;
using graph::Node;

using namespace llvm;

// http://llvm.org/docs/doxygen/html/classllvm_1_1InstVisitor.html
class Visitor : public InstVisitor<Visitor, ErrorCode> {
public:
    ConditionalAnalyzer conditionalAnalyzer;

    // http://llvm.org/docs/LangRef.html#store-instruction
    ErrorCode visitStoreInst(StoreInst &I) {
        Value *op1 = I.getOperand(0); // value to be stored
        Value *op2 = I.getOperand(1); // place to store the value

        // There are 3 different cases we'd like to detect:
        //  - CASE A: op1 is a constant value, either NULL or some NON-NULL.
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
        //            +-+ [const value, unknown dest]
        //            A
        //          +-|-+ [non-const value, unknown dest]
        //          | |
        //          B +-+ [const value, known dest]     +-+
        //          |                                     C
        //          +---+ [non-const value, known dest] +-+
        //
        // For that reason we split up the handling of this instruction as CASE 1-5.

        // CASE 1: We first detect whether the destination is known to us (CASE C).
        // If we know that it is NIL, then we report an error, regardless of what
        // op1 is (i.e. regardless of CASE A or B).
        if (graph.contains(op2) && graph.get(op2)->derefIsError()) {
            return handleDerefError(graph.get(op2));
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
                Node *leaf = graph.insert(Node::newLeafNode(graph::NIL));
                graph.insert(op2, Node::newRefNode(leaf));
            }
            else {
                Node *leaf = graph.insert(Node::newLeafNode(graph::NON_NIL));
                graph.insert(op2, Node::newRefNode(leaf));
            }
        }
        // CASE 4: A non-constant value is stored in some register, that is,
        // we store a reference to some other object in op2.
        //    - CASE 4.1: We have information about the value stored (e.g. another pointer value).
        //    - CASE 4.2: We don't have information (e.g. reference to some non-pointer value).
        else {
            if (graph.contains(op1)) {
                Node *referenced = graph.get(op1);
                graph.insert(op2, Node::newRefNode(referenced));
            } else {
                Node *referenced = graph.insert(op1, Node::newLeafNode(graph::DONT_KNOW)); // TODO too conservative?
                graph.insert(op2, Node::newRefNode(referenced));
            }
        }

        return OK;
    }

    // http://llvm.org/docs/LangRef.html#load-instruction
    ErrorCode visitLoadInst(LoadInst &I) {
        Value *op = I.getOperand(0);

        // CASE 1: we have information about the operand being dereferenced. First, we
        // check whether the derefercing is an error. If it is, then we return an error.
        // In the other case, we make this instruction point to the referenced node of the
        // node to which the operand points.
        if (graph.contains(op)) {
            Node *n = graph.get(op);
            if (n->derefIsError()) {
                return handleDerefError(I, n);
            } else if (n->isRef()) {
                Node *deref = n->refPtr()->getReferenced();
                graph.insert(&I, deref);
            } else {
                Node *newLeaf = graph.insert(Node::newLeafNode(graph::DONT_KNOW));
                n->transformToRefNode(newLeaf);
                graph.insert(&I, newLeaf);
            }
        }

        return OK;
    }

    // http://llvm.org/docs/LangRef.html#getelementptr-instruction
    ErrorCode visitGetElementPtrInst(GetElementPtrInst &I) {
        // Nothing is dereferenced by a GetElementPtrInstr -> no NULL_DEREF here
        // http://llvm.org/docs/GetElementPtr.html#what-is-dereferenced-by-gep

        Value *op = I.getPointerOperand();

        if (!graph.contains(op)) {
            graph.insert(op, Node::newLeafNode(graph::DONT_KNOW));
        }

        int64_t offset = -1;
        for (Use *u = I.op_begin()+1; u < I.op_end(); ++u) { // hack to find a reasonable offset
            ConstantInt *c = dyn_cast<ConstantInt>(u->get());
            if (c != NULL && offset == -1) offset = 0;
            if (c != NULL) offset += c->getSExtValue();
        }

        // In the case where we don't have a proper index value (it is -1),
        // we store that we don't know: any number of indices can be mapped
        // to -1 (see example 'array_unknown_indices').
        if (offset == -1) {
            graph.insert(&I, Node::newLeafNode(graph::DONT_KNOW));
        } else {
            Node *offsetNode = graph.getOffset(op, offset);
            graph.insert(&I, offsetNode);
        }

        return OK;
    }

    // http://llvm.org/docs/LangRef.html#llvm-memcpy-intrinsic
    ErrorCode visitMemCpyInst(MemCpyInst &I) {
        Value *source = I.getSource();
        Value *dest = I.getDest();

        // Just check whether the source and destination are known to be NULL.
        if ((graph.contains(source) && graph.get(source)->derefIsError())
                || (graph.contains(dest) && graph.get(dest)->derefIsError())) {
            return NULL_DEREF;
        }

        return OK;
    }

    ErrorCode visitInstruction(Instruction &I) {
        return OK;
    }

    ErrorCode visitBranchInst(BranchInst &I) {
        conditionalAnalyzer.visitBranchInst(I);
        return OK;
    }

    std::string dump() {
        return graph.dump();
    }

private:

    ErrorCode handleDerefError(Node *n) {
        switch (n->status()) {

        case NIL: return NULL_DEREF; // NULL is being dereferenced
        case UNDEFINED: return UNDEFINED_DEREF; // The result of a NULL deref is being dereferenced

        default: return OK;
        }
    }

    ErrorCode handleDerefError(Instruction &I, Node *n) {
        graph.insert(&I, Node::newLeafNode(graph::UNDEFINED));
        return handleDerefError(n);
    }

    Graph graph;
};

#endif // INST_VISITOR_H
