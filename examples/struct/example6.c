/*
Dereference of two pointers inside a struct using a pointer to that struct. Accessing the struct and dereferencing the one that is not null should work, dereferencing the null one should crash
*/

struct A { int *ptr1; int *ptr2; };

int main() {
    int value1 = 5;
    struct A a;
    struct A *ptr = &a;

    ptr->ptr1 = &value1;
    ptr->ptr2 = 0;

    int value2 = *(ptr->ptr1);
    int value3 = *(ptr->ptr2);
}
