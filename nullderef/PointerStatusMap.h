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
        std::string s;
        std::string type_str;
        llvm::raw_string_ostream rso(type_str);
        switch (type) {
        case LLVM_VALUE:
            s.append("LLVM_VALUE: ");
            s.append("VALUE=");
            this->value->print(rso);
            s.append(rso.str());
            s.append("; ");
            break;
        case STRUCT_FIELD:
            s.append("STRUCT_FIELD: ");
            s.append("VALUE=");
            this->value->print(rso);
            s.append(rso.str());
            s.append("; ");
            s.append("FIELDNO=");
            s.append(std::to_string(fieldno));
            break;
        default:
            break;
        }
        s.append("\n");
        return s;
    }
};

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
    PURE,
    IMITATION,
    REFERENCE
};

class PointerStatus {
    PointerStatusType type;
    PointerStatusValue statusValue; // only for PURE
    PointerStatus *parent; // only for REFERENCE and IMITATION

    PointerStatus(PointerStatusType type, PointerStatusValue statusValue, PointerStatus *parent)
        : type(type), statusValue(statusValue), parent(parent) {}

public:
    // Nonsense constructor to be able to store values in map
    PointerStatus(const PointerStatus &other) : PointerStatus(other.type, other.statusValue, other.parent) {}

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
        case REFERENCE: return parent->getStatus();
        case PURE: return statusValue;
        }
    }

    void setStatus(PointerStatusValue status) {
        // If this is a pure status, then update the statusValue
        // If this is an imitation, then update the original
        // If this is a reference status, then change this reference's type to PURE and set the value
        switch (type) {
        case IMITATION:
            this->parent->setStatus(status);
            break;
        case REFERENCE:
            this->type = PURE;
            this->statusValue = status;
            this->parent = NULL;
            break;
        case PURE: // fall through
        default:
            this->statusValue = status;
            break;
        }
    }

    int depth() const {
        switch (type) {
        case IMITATION: return this->parent->depth();
        case REFERENCE: return 1 + this->parent->depth();
        case PURE: return 0;
        }
    }

    /// If we dereference this, do we get a null dereference?
    bool isNullDeref() const {
        return this->statusValue == NIL && this->depth() == 0;
    }

    bool hasParent() { return this->getParent() != NULL; }

    /// Get the PointerStatus this pointer status refers to, or NULL if there is no such parent
    PointerStatus *getParent() {
        switch (type) {
        case IMITATION: return parent->getParent();
        case REFERENCE: return parent;
        case PURE: return NULL;
        }
    }

    void setParent(PointerStatus *parent) {
        switch (type) {
        case IMITATION: this->parent->setParent(parent); break;
        case REFERENCE: this->parent = parent; break;
        case PURE: throw "setParent() not allowed on PointerStatusType of PURE";
        }
    }

    std::string prettyPrint() {
        std::stringstream ss;
        switch (type) {
        case PURE:
            ss << "PURE: ID=";
            ss << ((size_t)this&0xffff);
            ss << "; STATUS=";
            switch (statusValue) {
            case NIL: ss << "NIL; "; break;
            case NON_NIL: ss << "NON_NIL; "; break;
            case DONT_KNOW: ss << "DONT_KNOW; "; break;
            default: ss << "???"; break;
            }
            break;
        case IMITATION:
            ss << "IMITATION: ID=";
            ss << (((size_t) this)&0xffff);
            ss << "; REFERENCE=";
            ss << (((size_t)this->parent)&0xffff);
            break;
        case REFERENCE:
            ss << "REFERENCE: ID=";
            ss << (((size_t) this)&0xffff);
            ss << "; PARENT=";
            ss << (((size_t)this->parent)&0xffff);
            break;
        default:
            ss << "JEEZ ERROR";
            break;
        }
        ss << "\n";
        return ss.str();
    }

};

class PointerStatusMap {
    std::unordered_map<PointerKey, PointerStatus*> map;
public:
    // We pass keys as value, make sure the type doesn't grown too large
    PointerStatus* get(PointerKey key) { return this->map[key]; }
    PointerStatus* get(Value *value) { return get(PointerKey::createLlvmKey(value)); }

    ~PointerStatusMap() {
        for (std::pair<PointerKey, PointerStatus*> p : this->map) {
            delete p.second;
        }
    }

    bool contains(PointerKey key) { return this->map.count(key); }
    bool contains(Value *value) { return this->contains(PointerKey::createLlvmKey(value)); }

    PointerStatus* put(PointerKey key, const PointerStatus& status) {
        PointerStatus *heapStatus = new PointerStatus(status);
        this->map[key] = heapStatus;
        return heapStatus;
    }
    PointerStatus* put(Value *value, const PointerStatus& status) { this->put(PointerKey::createLlvmKey(value), status); }

    void dump() {
        for (std::pair<PointerKey, PointerStatus*> p : this->map) {
            errs() << "key:   ";
            errs() << p.first.prettyPrint();
            errs() << "value: ";
            if (p.second == NULL) {
                errs() << "NULL\n";
            } else {
                errs() << p.second->prettyPrint();
            }
            errs()  << "\n";
        }
    }
};

#endif // POINTER_STATUS_MAP_H
