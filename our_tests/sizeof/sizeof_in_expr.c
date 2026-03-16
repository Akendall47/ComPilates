/* sizeof can be used inside an expression, not just returned directly.
   Should compute 4 + 1 = 5 */
int f()
{
    int n;
    char c;
    n = sizeof(int) + sizeof(char);
    return n;
}
