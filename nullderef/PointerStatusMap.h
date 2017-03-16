#ifndef POINTER_STATUS_MAP_H
#define POINTER_STATUS_MAP_H

#include <memory>
#include <functional>
#include <unordered_map>
#include <llvm/IR/Value.h>

class PointerKey {
public:
    bool operator ==(const PointerKey other) const;
    virtual std::size_t hash() const noexcept;
};

class LLVMValueKey : public PointerKey {
    const llvm::Value* value;

public:
    std::size_t hash() const noexcept override;
};

class StructFieldKey : public PointerKey {
    llvm::Value* strct;
    int field_no;
public:
    std::size_t hash() const noexcept override;
};

namespace std {
    template<> struct hash<PointerKey> {
        std::size_t operator()(PointerKey const& key) const noexcept {
            return key.hash();
        }
    };
}

class PointerStatus {
public:
    static const short NIL = 1;
    static const short NON_NIL = 2;
    static const short DONT_KNOW = 1 | 2;

    virtual short status() const;
    virtual int depth() const;
    virtual bool isNullDeref() const;
};

class PureValue : public PointerStatus {
    const short status_value;

public:
    PureValue();
    PureValue(short status);

    short status() const override;
    int depth() const override;
};

class ImitationValue : public PointerStatus {
    const PointerStatus& imitated;

public:
    ImitationValue(const PointerStatus& imitated);

    short status() const override;
    int depth() const override;
};

class ReferenceValue : public PointerStatus {
    const PointerStatus& ref;

public:
    ReferenceValue(const PointerStatus& ref);

    short status() const override;
    int depth() const override;
};

class TrackerMap {
    std::unordered_map<PointerKey, PointerStatus> map;
public:
    /* We pass keys and statusses as value, make sure our types don't grown too large */
    PointerStatus get(PointerKey key);
    bool contains(PointerKey key);
    void put(PointerKey key, PointerStatus value);
};


#endif // POINTER_STATUS_MAP_H
