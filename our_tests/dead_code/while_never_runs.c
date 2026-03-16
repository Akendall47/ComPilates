/* while(0) never runs even once, so the body is completely dead.
   count should stay 6 — the loop messing with it should never happen. */
int f()
{
    int count;
    int junk;
    count = 6;
    junk = 0;
    while (0) {
        count = 99;
        junk = junk + 1;
    }
    return count;
}
