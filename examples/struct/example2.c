/*
Set the element of a struct when it's pointer is null
*/

struct A {
    int value;
};

int main() {
    struct A *ptr = 0;
    ptr->value = 77;
}
