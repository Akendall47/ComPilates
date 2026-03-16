/* Both the if and else return early, so anything after the if-else is unreachable.
   The stray code at the bottom should never be hit. */
int f(int n)
{
    int val;
    int extra;
    extra = 5;
    if (n > 0) {
        val = n + extra;
        return val;
    } else {
        val = extra - n;
        return val;
    }
    val = 777;
    return val;
}
