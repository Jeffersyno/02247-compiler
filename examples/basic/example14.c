/*
Dereference of a pointer stored inside an array
*/

int main() {
    int* pointer = 0;
    int* a[1] = {pointer};
    int value = *a[0];
}
