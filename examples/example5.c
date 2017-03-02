
struct A { int x; int* value; };

void example5() {
    struct A a;
    a.value = 0;
    int b = *(a.value);
}
