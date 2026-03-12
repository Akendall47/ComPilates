/* >> on unsigned should zero-fill (srl), not sign-extend (sra) */
unsigned int f() {
    unsigned int x = 0x80000000;
    return x >> 1;
}
