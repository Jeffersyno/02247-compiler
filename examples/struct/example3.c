/*
Access to the value pointed by a null dereference integer pointer declared inside a struct
*/

struct A { 
	int* value; 
};

int main() {
    struct A a;
    a.value = 0;
    int b = *(a.value);
}
