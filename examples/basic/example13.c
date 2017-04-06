/*
Cast an integer with value 0 to a pointer, getting a null pointer and dereferencing it after
*/

int main() {
    long int i = 0;
    long int *ptr = (long int *) i;
    long int value = *ptr;
}
