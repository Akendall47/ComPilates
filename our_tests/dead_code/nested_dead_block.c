/* The outer if(1) always runs, but the inner if(0) never does.
   total should stay 11 — the dead inner block should be completely skipped. */
int f()
{
    int total;
    int step;
    step = 4;
    total = step + 7;
    if (1) {
        step = step + 1;
        if (0) {
            total = 888;
            step = 0;
        }
    }
    return total;
}
