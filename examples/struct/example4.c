struct A { int v; int* ptr; };

int main() {
    struct A a;
    struct A *p = &a;
    p->ptr = 0;
    int i = *(p->ptr);
}
