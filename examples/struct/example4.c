/*
Access to the value pointed by a null dereferenced pointer that is declared inside of a struct
*/

struct A { int v; int* ptr; };

int main() {
    struct A a;
    struct A *p = &a;
    p->ptr = 0;
    int i = *(p->ptr);
    int j = *(a.ptr);
}
