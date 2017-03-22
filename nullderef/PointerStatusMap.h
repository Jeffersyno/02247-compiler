#ifndef POINTER_STATUS_MAP_H
#define POINTER_STATUS_MAP_H

#include <memory>
#include <functional>
#include <unordered_map>
#include <llvm/IR/Value.h>
#include <llvm/ADT/Hashing.h>

using namespace llvm;

class PointerKey {
public:
    bool operator ==(const PointerKey other) const { return this->hash() == other.hash(); }
    virtual size_t hash() const noexcept { return hash_value(0); }
    virtual const Value *llvmValue() const;
};

class LLVMValueKey: public PointerKey {
    const Value *value;

public:
    LLVMValueKey(Value *value) : value(value) {}
    size_t hash() const noexcept override { return hash_value(this->value); }
    const Value *llvmValue() const { return value; }
};

class StructFieldKey : public PointerKey {
    const Value* strct;
    const int field_no;
public:
    size_t hash() const noexcept override { return (31 + hash_value(this->strct)) * 23 + (this->field_no << 2); }
    const Value *llvmValue() const { return strct; }
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
 *              {}
 */
class PointerStatus {
public:
    static const short NIL = 1;
    static const short NON_NIL = 2;
    static const short DONT_KNOW = 3;

    virtual short status() const { return 0; }
    virtual int depth() const { return 0; }
    virtual bool isNullDeref() const { return true; }
};

class PureStatus : public PointerStatus {
    const short status_value;

public:
    PureStatus(short status) : status_value(status) {}

    short status() const override { return this->status_value; }
    int depth() const override { return 0; }
};

class ImitationStatus : public PointerStatus {
    const PointerStatus& imitated;

public:
    ImitationStatus(const PointerStatus& imitated);

    short status() const override { return this->imitated.status(); }
    int depth() const override { return this->imitated.depth(); }
};

class ReferenceStatus : public PointerStatus {
    const PointerStatus& ref;

public:
    ReferenceStatus(const PointerStatus& ref);

    short status() const override { return this->ref.status(); }
    int depth() const override { return 1 + this->ref.depth(); }
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
            errs() <<  "hello from " << p.first.llvmValue() << " with status " << p.second.status() << "\n";
        }
    }
};

#endif // POINTER_STATUS_MAP_H
