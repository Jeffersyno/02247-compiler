#include <memory>
#include <llvm/ADT/Hashing.h>

#include "PointerStatusMap.h"


bool PointerKey::operator ==(PointerKey &other) const
{
    // TODO fix
    return this->hash() == other.hash();
}

std::size_t LLVMValueKey::hash() const noexcept
{
    return llvm::hash_value(this->value);
}

std::size_t StructFieldKey::hash() const noexcept
{
    return (31 + llvm::hash_value(this->strct)) * 23 + (this->field_no << 2);
}

short PureValue::status() const
{
    return this->status_value;
}

int PureValue::depth() const
{
    return 0;
}

short ImitationValue::status() const
{
    return this->imitated.status();
}

int ImitationValue::depth() const
{
    return this->imitated.depth();
}

short ReferenceValue::status() const
{
    return this->ref.status();
}

int ReferenceValue::depth() const
{
    return 1 + this->ref.depth();
}

PointerStatus &TrackerMap::get(const PointerKey &key) const
{
    // TODO implement
}
