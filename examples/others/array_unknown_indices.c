/*
This example should not fail but it does, because we don't have information
about the indices used, so we consider them "the same" (-1).
Check out constant folding pass
*/

int main() {
    int i = 5;
    int j = 6;
    int *list[] = {&i, &j, &j+1};
    int **a = &list[0];

    a[j] = &i;
    a[i] = 0;

    int result = *a[j];
}
