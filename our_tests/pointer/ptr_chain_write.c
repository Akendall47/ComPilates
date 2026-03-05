int f() {
    int x;
    int *p;
    int **pp;
    x = 0;
    p = &x;
    pp = &p;
    **pp = 55;
    return x;
}
