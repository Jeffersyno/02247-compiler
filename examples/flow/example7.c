
int main() {
    int cond = 5;
    int *ptr = &cond;
    if (cond > 9) { // some condition
        ptr = 0;
    } 
    cond = *ptr;
}
