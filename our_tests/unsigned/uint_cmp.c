/* 0xFFFFFFFF is > 1 as unsigned, but < 1 as signed (-1) */
unsigned int f() {
    unsigned int big = 0xFFFFFFFF;
    unsigned int small = 1;
    if (big > small) return 1;
    return 0;
}
