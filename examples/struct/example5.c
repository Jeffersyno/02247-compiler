
struct A { int *q1; int *q2; };

int main() {
    int i = 5;
    struct A a;
    struct A *p;

    p->q1 = &i;
    p->q2 = 0;

    int j = *(p->q1);
}
