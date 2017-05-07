/*
Get the struct linked with a null dereferenced pointer
*/

struct A {
    int value;
};

int main() {
    struct A *ptr1 = 0;
    struct A ptr2 = *ptr1;
}
