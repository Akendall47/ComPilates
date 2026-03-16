/* if(1) is always true, so the else branch can never be reached.
   result should be 27 — the else path should never happen. */
int f()
{
    int result;
    int tmp;
    tmp = 3;
    if (1) {
        result = 27;
    } else {
        result = tmp * 999;
    }
    return result;
}
