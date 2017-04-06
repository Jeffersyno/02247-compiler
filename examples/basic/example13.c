/*
Cast an integer with value 0 to a pointer, getting a null pointer and dereferencing it after.
This is not in the scope of the project, because the program will also crash using other integer value, not only 0
*/

int main() {
    long int i = 0;
    long int *ptr = (long int *) i;
    long int value = *ptr;
}
