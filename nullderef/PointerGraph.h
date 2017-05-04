#ifndef POINTER_GRAPH_H
#define POINTER_GRAPH_H 1

#include <string>
#include <vector>
#include <iomanip>
#include <unordered_map>

#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

using std::string;
using std::stringstream;
using std::vector;
using std::unordered_map;
using namespace llvm;

namespace graph {

class Node; // forward declaration

enum NodeType {
    LEAF_NODE,
    REFERENCE_NODE
};

enum LeafNodeType {
    NIL = 1,
    NON_NIL = 2,
    DONT_KNOW = NIL | NON_NIL,
    UNDEFINED = 8
};

struct LeafNode {
    LeafNodeType type;
    LeafNode(LeafNodeType type) : type(type) {}
};

struct RefNode {
    Node *referenced;
    RefNode(Node* referenced) : referenced(referenced) {}

    Node *getReferenced() {
        return referenced;
    }
};

class Node {
    NodeType type;
    union {
        LeafNode leaf;
        RefNode ref;
    };

    Node(LeafNode leaf) : type(LEAF_NODE), leaf(leaf) {}
    Node(RefNode ref) : type(REFERENCE_NODE), ref(ref) {}

public:
    Node(const Node &other) : type(other.type) {
        switch (type) {
        case LEAF_NODE: leaf = other.leaf; break;
        case REFERENCE_NODE: ref = other.ref; break;
        default: throw "unreachable";
        }
    }

    static Node newLeafNode(LeafNodeType type) { return Node(LeafNode(type)); }
    static Node newRefNode(Node* referenced) { return Node(RefNode(referenced)); }

    bool isLeaf() const { return type == LEAF_NODE; }
    bool isRef() const { return type == REFERENCE_NODE; }

    LeafNode *leafPtr() { return isLeaf() ? &leaf : nullptr; }
    RefNode *refPtr() { return isRef() ? &ref : nullptr; }

    bool derefIsError() const {
        return type == LEAF_NODE  && (leaf.type == NIL || leaf.type == UNDEFINED);
    }

    /// Turn this node into a REF node point to the given node.
    void transformToRefNode(Node *referenced) {
        if (type != LEAF_NODE) throw "Transforming REF to REF node.";

        type = REFERENCE_NODE;
        ref.referenced = referenced;
    }

    int32_t depth() const {
        switch (type) {
        case LEAF_NODE: return 0;
        case REFERENCE_NODE: return 1 + ref.referenced->depth();
        default: throw "unreachable";
        }
    }

    LeafNodeType status() const {
        switch (type) {
        case LEAF_NODE: return leaf.type;
        case REFERENCE_NODE: return NON_NIL;
        default: throw "unreachable";
        }
    }

    string dumpHexId() const {
        // Try to generate a psuedo-random number from the pointer
        // http://stackoverflow.com/a/4825477/2900494
        stringstream ss;
        uint64_t id = 5381;
        id = (((id << 5) + id) + (((uint64_t) this) >>  0)) & 0xFFFF;
        id = (((id << 5) + id) + (((uint64_t) this) >>  8)) & 0xFFFF;
        id = (((id << 5) + id) + (((uint64_t) this) >> 16)) & 0xFFFF;
        id = (((id << 5) + id) + (((uint64_t) this) >> 32)) & 0xFFFF;
        id = (((id << 5) + id) + (((uint64_t) this) >> 40)) & 0xFFFF;
        id = (((id << 5) + id) + (((uint64_t) this) >> 48)) & 0xFFFF;
        id = (((id << 5) + id) + (((uint64_t) this) >> 56)) & 0xFFFF;
        ss << '<' << std::hex << std::setw(4) << std::setfill('0') << id << '>';
        return ss.str();
    }

    string dump() const {
        stringstream ss;

        ss << dumpHexId();

        switch (type) {
        case LEAF_NODE:
            ss << " LEAF/";
            switch (leaf.type) {
            case NIL: ss << "NIL"; break;
            case NON_NIL: ss << "NON_NIL; "; break;
            case DONT_KNOW: ss << "DONT_KNOW; "; break;
            case UNDEFINED: ss << "UNDEFINED"; break;
            default: ss << "???"; break;
            }
            if (derefIsError()) { ss << " (!)"; }
            break;

        case REFERENCE_NODE:
            ss << " REF OF ";
            ss << ref.referenced->dumpHexId();
            ss << " at depth " << depth();
            break;

        default:
            ss << "ERROR (Node::dump)";
            break;
        }

        return ss.str();
    }
};

struct OffsetNodeKey {
    Node *original;
    int64_t offset;

    OffsetNodeKey(Node *original, int64_t offset)
        : original(original), offset(offset) {}

    friend struct std::hash<OffsetNodeKey>;
    friend bool operator ==(const OffsetNodeKey&, const OffsetNodeKey&);
};

bool operator==(const graph::OffsetNodeKey &lhs, const graph::OffsetNodeKey &rhs) {
    return lhs.original == rhs.original && lhs.offset == rhs.offset;
}

} // namespace graph


namespace std {

template<> struct hash<graph::OffsetNodeKey> {
    size_t operator()(graph::OffsetNodeKey const& key) const noexcept {
        return (((size_t) key.original) ^ ((key.offset+1)<<1));
    }
};

} // namespace std


namespace graph {

class Graph {
    vector<Node*> allocations;
    unordered_map<OffsetNodeKey, Node*> offsetNodes;
    ValueMap<Value*, Node*> valueMap;

public:
    Graph() {}

    ~Graph() {
        // free graph nodes
        while (!allocations.empty()) {
            delete allocations.back();
            allocations.pop_back();
        }
    }

    /// Insert a new node in the graph without creating an entry with an LLVM value.
    Node *insert(Node node) {
        Node *heap = new Node(node);
        allocations.push_back(heap);
        return heap;
    }

    /// Insert a new node in the graph and make the given value an entry point into the graph.
    Node *insert(Value *value, Node node) {
        Node *heap;
        if (contains(value)) {
            heap = get(value);
            *heap = node;
        } else {
            heap = insert(node);
            valueMap[value] = heap;
        }
        return heap;
    }

    /// Insert a new entry point; reuse given node.
    void insert(Value *value, Node *node) {
        valueMap[value] = node;
    }

    Node *get(Value *value) {
        return valueMap[value];
    }

    /// Get the offset node or creates and returns a new LEAF node with
    /// the same status as the given value's status.
    Node *getOffset(Value *value, int64_t offset) {
        if (!contains(value)) throw "Creating offset of something I don't know";

        Node *base = get(value);
        if (containsOffset(base, offset))
            return offsetNodes[OffsetNodeKey(base, offset)];

        Node *leaf = insert(Node::newLeafNode(base->status())); // take status of base
        offsetNodes[OffsetNodeKey(base, offset)] = leaf;
        return leaf;
    }

    bool contains(Value *value) {
        return valueMap.count(value) != 0;
    }

    bool containsOffset(Value *value, int64_t offset) {
        if (!contains(value)) return false;
        return containsOffset(get(value), offset);
    }

    bool containsOffset(Node *base, int64_t offset) {
        return offsetNodes.count(OffsetNodeKey(base, offset)) != 0;
    }

    string dump(Value *value) {
        string s;
        llvm::raw_string_ostream rso(s);
        value->print(rso);
        string r = rso.str();

        auto n = r.find("getelementptr inbounds");
        if (n != std::string::npos) { r.replace(n, 22, "GEP"); } // shorten the name a bit

        return r;
    }

    string dump() {
        std::stringbuf buf;
        std::ostream os(&buf);

        os << "\nNODES IN GRAPH:\n";
        for (Node *n : allocations) {
            os << " - " << n->dump() << "\n";
        }

        os << "\nENTRY POINTS INTO GRAPH:\n";
        for (auto p : valueMap) {
            os << " - " << std::left << std::setw(60) << dump(p.first);
            os << " => " << p.second->dump();
            os << "\n";
        }

        os << "\nDERIVED OFFSET NODES\n";
        for (auto p : offsetNodes) {
            os << " - " << p.second->dumpHexId();
            os << " = (" << p.first.original->dumpHexId() << ", " << p.first.offset << ")";
            os << "\n";
        }

        return buf.str();
    }

};



} // namespace graph

#endif // POINTER_GRAPH_H
