/* Once we hit the first return, nothing after it should ever run.
   The second return is just sitting there doing nothing. */
int f()
{
    int a;
    int b;
    a = 6;
    b = 13;
    return a + b;
    return 999;
}
