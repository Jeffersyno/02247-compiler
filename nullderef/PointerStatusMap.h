#ifndef POINTER_STATUS_MAP_H
#define POINTER_STATUS_MAP_H

#include <unordered_map>
#include <llvm/IR/Value.h>
#include <llvm/ADT/Hashing.h>
#include <sstream>

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

    std::string prettyPrint() {
        std::string s = "TYPE= ";
        if(type == PointerKeyType::LLVM_VALUE) { s.append("LLVM_VALUE; "); } else {
            s.append("STRUCT_FIELD; ");
        }
        s.append("VALUE=");
        value->dump();
        s.append("FIELDNO=");
        return s;
    }
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
    IMITATION,
    REFERENCE
};

class PointerStatus {
    PointerStatusType type;
    PointerStatusValue statusValue; // only for PURE
    PointerStatus *reference; // only for REFERENCE and IMITATION

    PointerStatus(PointerStatusType type, PointerStatusValue status, PointerStatus *reference)
        : type(type), statusValue(status), reference(reference) {}

public:
    // nonsense constructor to be able to store values in map
    PointerStatus() : PointerStatus(NONSENSE, DONT_KNOW, NULL) {}

    static PointerStatus createPure(PointerStatusValue status) {
        return PointerStatus(PURE, status, NULL);
    }

    static PointerStatus createImitation(PointerStatus *ps) {
        return PointerStatus(IMITATION, DONT_KNOW, ps);
    }

    static PointerStatus createReference(PointerStatus *ps) {
        return PointerStatus(REFERENCE, DONT_KNOW, ps);
    }

    PointerStatusValue getStatus() const {
        switch (type) {
        case IMITATION: // fall through
        case REFERENCE: return reference->getStatus();
        case NONSENSE: throw "getStatus() not allowed on PointerStatusType of NONSENSE";
        case PURE: // fall through
        default: return statusValue;
        }
    }

    void setStatus(PointerStatusValue status) {
        // if this is a pure status, then update the statusValue
        // if this is an immutated, then update the original
        // if this is a reference status, then change this reference's type to PURE and set the value
        switch (type) {
        case IMITATION:
            reference->setStatus(status); break;
        case REFERENCE:
            this->type = PURE;
            this->statusValue = status;
            this->reference = NULL;
            break;
        case NONSENSE: 
            throw "setStatus() not allowed on PointerStatusType of NONSENSE";
        case PURE: // fall through
        default:
            this->statusValue = status;
            break;
        }
    }

    int depth() const {
        switch (type) {
        case IMITATION: return reference->depth();
        case REFERENCE: return 1 + reference->depth();
        case NONSENSE: throw "depth() not allowed on PointerStatusType of NONSENSE";
        case PURE: // fall through
        default: return 0;
        }
    }

    /// If we dereference this, do we get a null dereference?
    bool isNullDeref() const {
        return this->statusValue == NIL && this->depth() == 0;
    }

    bool hasParent() { return this->getParent() != NULL; }

    /// Get the PointerStatus this pointer status refers to, or NULL if there is no such parent.
    PointerStatus *getParent() {
        switch (type) {
        case IMITATION: return reference->getParent();
        case REFERENCE: return reference;
        case NONSENSE: throw "getParent() not allowed on PointerStatusType of NONSENSE";
        case PURE: // fall through
        default: return NULL;
        }
    }

    void setParent(PointerStatus *parent) {
        switch (type) {
        case IMITATION: reference->setParent(parent); break;
        case REFERENCE: reference = parent; break;
        case NONSENSE: throw "setParent() not allowed on PointerStatusType of NONSENSE";
        case PURE: // fall through
        default:
            throw "setParent() not allowed on PointerStatusType of PURE";
        }
    }

    std::string prettyPrint() {
        std::string s = "TYPE=";
        std::stringstream ss;
        switch (this->type) {
        case PURE:
            s.append("PURE; ");
            s.append("STATUSVALUE=");
            switch (this->statusValue) {
                case NIL: s.append("NIL; "); break;
                case NON_NIL: s.append("NON_NIL; "); break;
                case DONT_KNOW: s.append("DONT_KNOW; "); break;
            default: break;
            }
            break;
        case IMITATION:
            s.append("IMITATION; ");
            s.append("REFERENCE=");
            ss << (((size_t)&reference)&0xffff);
            s.append(ss.str());
            break;
        case REFERENCE:
            s.append("REFERENCE; ");
            s.append("REFERENCE=");
            ss << (((size_t)&reference)&0xffff);
            s.append(ss.str());
            break;
        default:
            break;
        }
        s.append("\n");
        return s;
    }

};

class PointerStatusMap {
    std::unordered_map<PointerKey, PointerStatus> map;
public:
    /* We pass keys and statuses as value, make sure our types don't grown too large */
    PointerStatus& get(PointerKey key) { return this->map[key]; }
    PointerStatus& get(Value *value) { return get(PointerKey::createLlvmKey(value)); }

    bool contains(PointerKey key) { return this->map.count(key); }
    bool contains(Value *value) { return this->contains(PointerKey::createLlvmKey(value)); }

    void put(PointerKey key, PointerStatus status) { this->map[key] = status; }
    void put(Value *value, PointerStatus status) { this->put(PointerKey::createLlvmKey(value), status); }

    void dump() {
        for (std::pair<PointerKey, PointerStatus> p : this->map) {
            errs() << "key:   ";
            p.first.getLlvmValue()->dump();
            errs() << "value: ";
            switch (p.second.getStatus()) {
            case NIL: errs() << "NIL"; break;
            case NON_NIL: errs() << "NON NIL"; break;
            case DONT_KNOW: errs() << "DONT KNOW"; break;
            default: errs() << "NONSENSE"; break;
            }

            errs()  << "\n";
        }
    }
};

#endif // POINTER_STATUS_MAP_H
