#ifndef POINTER_STATUS_MAP_H
#define POINTER_STATUS_MAP_H

//#include <memory>
#include <unordered_map>
#include <llvm/IR/Value.h>
#include <llvm/ADT/Hashing.h>

using namespace llvm;

enum PointerKeyType {
    LLVM_VALUE,
    STRUCT_FIELD
};

class PointerKey {
    PointerKeyType type;
    Value *value;
    int fieldno;

    //PointerKey() : PointerKey(PointerKeyType::NONSENSE, NULL, 0) {}
    PointerKey(PointerKeyType type, Value *value, int fieldno)
        : type(type), value(value), fieldno(fieldno) {}

public:
    static PointerKey createLlvmKey(Value *value) {
        return PointerKey(LLVM_VALUE, value, 0);
    }

    static PointerKey createStructFieldKey(Value *value, int fieldno) {
        return PointerKey(STRUCT_FIELD, value, fieldno);
    }

    bool operator ==(const PointerKey other) const {
        return this->type == other.type
                && this->value == other.value
                && this->fieldno == other.fieldno;
    }

    size_t hash() const noexcept {
        size_t h = 13;
        h = 23*h + this->type;
        h = 23*h + this->fieldno;
        h = 23*h + hash_value(this->value);
        return h;
    }

    Value *getLlvmValue() const { return this->value; }
};

// TODO: unused class, remove? This is maybe some trush form initial development?
// It's the hash function used by the map, don't remove.
namespace std {
    template<> struct hash<PointerKey> {
        size_t operator()(PointerKey const& key) const noexcept { return key.hash(); }
    };
}

/**
 * The PointerStatus type stores information about a pointer type.
 *
 *         {NIL, NON_NIL}
 *           //     \\
 *       {NIL}     {NON_NIL}
 *           \\     //
 *              {} --> don't care about this
 */
enum PointerStatusValue {
    NIL = 1,
    NON_NIL = 2,
    DONT_KNOW = NIL | NON_NIL
};

enum PointerStatusType {
    NONSENSE,
    PURE,
    IMMITATION,
    REFERENCE
};

class PointerStatus {
    PointerStatusType type;
    PointerStatusValue statusValue;
    PointerStatus *reference;

    PointerStatus(PointerStatusType type, PointerStatusValue status, PointerStatus *reference)
        : type(type), statusValue(status), reference(reference) {}

public:
    // nonsense constructor to be able to store values in map
    PointerStatus() : PointerStatus(NONSENSE, DONT_KNOW, NULL) {}

    static PointerStatus nil() { return PointerStatus(PURE, NIL, NULL); }
    static PointerStatus nonNil() { return PointerStatus(PURE, NON_NIL, NULL); }
    static PointerStatus dontKnow() { return PointerStatus(PURE, DONT_KNOW, NULL); }

    static PointerStatus createImmitation(PointerStatus *ps) {
        return PointerStatus(IMMITATION, ps->statusValue, ps);
    }

    static PointerStatus createReference(PointerStatus *ps) {
        return PointerStatus(REFERENCE, ps->statusValue, ps);
    }

    PointerStatusValue getStatus() const {
        switch (type) {
        case IMMITATION: // fall through
        case REFERENCE: return reference->getStatus();
        case PURE: // fall through
        default: return statusValue;
        }
    }

    int depth() const {
        switch (type) {
        case IMMITATION: return reference->depth();
        case REFERENCE: return 1 + reference->depth();
        case PURE: // fall through
        default: return 0;
        }
    }

    /// If we dereference this, do we get a null dereference?
    bool isNullDeref() const {
        switch (type) {
        case PURE: return this->statusValue == NIL;
        case IMMITATION:  return reference->isNullDeref();
        case REFERENCE: // fall through
        default: return false;
        }
    }

    /// Get the status this status refers to, or NULL in the case of a pure value.
    PointerStatus *dereference() {
        switch (type) {
        case IMMITATION: return reference->dereference();
        case REFERENCE: return reference;
        case PURE: // fall through
        default: return NULL;
        }
    }

};

class PointerStatusMap {
    std::unordered_map<PointerKey, PointerStatus> map;
public:
    /* We pass keys and statuses as value, make sure our types don't grown too large */
    PointerStatus get(PointerKey key) { return this->map[key]; }
    bool contains(PointerKey key) { return this->map.count(key); }
    void put(PointerKey key, PointerStatus value) { this->map[key] = value; }
    void dump() {
        for (std::pair<PointerKey, PointerStatus> p : this->map) {
            errs() << "key:   ";
            p.first.getLlvmValue()->dump();
            errs() << "value: " << p.second.getStatus() << "\n";
        }
    }
};

#endif // POINTER_STATUS_MAP_H
