/*
Dereference of an integer pointer inside a struct
*/

struct A { 
	int* value; 
};

int main() {
    struct A a;
    a.value = 0;
    int b = *(a.value);
}
