#ifndef POINTER_STATUS_MAP_H
#define POINTER_STATUS_MAP_H

#include <memory>
#include <functional>
#include <unordered_map>
#include <llvm/IR/Value.h>
#include <llvm/ADT/Hashing.h>

// TODO: unused class, remove? This is maybe some trush form initial development?
namespace std {
    template<> struct hash<PointerKey> {
        size_t operator()(PointerKey const& key) const noexcept { return key.hash(); }
    };
}

class PointerKey {
public:
    bool operator ==(const PointerKey other) const { return this->hash() == other.hash(); }
    virtual std::size_t hash() const noexcept { return llvm::hash_value(0); }
};

class LLVMValueKey: public PointerKey {
    const llvm::Value* value;

public:
    std::size_t hash() const noexcept override { return llvm::hash_value(this->value); }
};

class StructFieldKey : public PointerKey {
    llvm::Value* strct;
    int field_no;
public:
    std::size_t hash() const noexcept override { return (31 + llvm::hash_value(this->strct)) * 23 + (this->field_no << 2); }
};

class PointerStatus {
public:
    static const short NIL = 1;
    static const short NON_NIL = 2;
    static const short DONT_KNOW = 1 | 2;

    virtual short status() const { return 0; }
    virtual int depth() const { return 0; }
    virtual bool isNullDeref() const { return true; }
};

class PureValue : public PointerStatus {
    const short status_value;

public:
    PureValue();
    PureValue(short status);

    short status() const override { return this->status_value; }
    int depth() const override { return 0; }
};

class ImitationValue : public PointerStatus {
    const PointerStatus& imitated;

public:
    ImitationValue(const PointerStatus& imitated);

    short status() const override { return this->imitated.status(); }
    int depth() const override { return this->imitated.depth(); }
};

class ReferenceValue : public PointerStatus {
    const PointerStatus& ref;

public:
    ReferenceValue(const PointerStatus& ref);

    short status() const override { return this->ref.status(); }
    int depth() const override { return 1 + this->ref.depth(); }
};

class TrackerMap {
    std::unordered_map<PointerKey, PointerStatus> map;
public:
    /* We pass keys and statuses as value, make sure our types don't grown too large */
    PointerStatus get(PointerKey key) { return this->map[key]; }
    bool contains(PointerKey key) { return this->map.count(key); }
    void put(PointerKey key, PointerStatus value) { this->map[key] = value; }
};

#endif // POINTER_STATUS_MAP_H
