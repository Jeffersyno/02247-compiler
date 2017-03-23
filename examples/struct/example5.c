
struct A { int x; int* value; };

int main() {
    struct A a;
    a.value = 0;
    int b = *(a.value);
}
