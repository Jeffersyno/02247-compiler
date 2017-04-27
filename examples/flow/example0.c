int* get_ptr();

int main() {
    int *ptr = 0;
    while (!(ptr = get_ptr())) {}
    *ptr;
}
