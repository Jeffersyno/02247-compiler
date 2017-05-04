struct A { int b; int *ptr; };
int main() {
    struct A a;
    a.ptr = 0;
    int val = *(a.ptr);
}
