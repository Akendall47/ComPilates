/* sizeof result used in an if condition — if sizeof(int) == 4 we return 1 */
int f()
{
    int result;
    result = 0;
    if (sizeof(int) == 4) {
        result = 1;
    }
    return result;
}
