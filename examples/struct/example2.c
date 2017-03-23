/*
Get the struct linked with a dereferenced pointer
*/

struct A {
    int value;
};

int main() {
    struct A *ptr1 = 0;
    struct A ptr2 = *ptr1;
}
