/*
Access of the element of a struct when the struct pointer is null
*/

struct A { 
	int value; 
};

int main() {
    struct A *x = 0;
    int value = x->value;
}
