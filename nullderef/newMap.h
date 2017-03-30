//
// Created by Andrés Cecilia on 23/3/17.
//

#include <vector>

#ifndef PROJECT_NEWMAP_H
#define PROJECT_NEWMAP_H

std::vector<Address> adresses;

struct Address {
    const AddressID id;
public:
    const Address value;

    Address(const AddressID id, const Address value) : id(id), value(value) { }

    bool operator==(const Address& other) const { return id == other.id; }
};

struct AddressID {
private:
    const unsigned int line;

public:
    AddressID(unsigned int line) : line(line) { }
    bool operator==(const AddressID& other) const { return line == other.line; }
};

#endif //PROJECT_NEWMAP_H
