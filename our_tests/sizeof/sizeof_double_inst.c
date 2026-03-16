/* sizeof on a double variable should also give 8 */
int f()
{
    double d;
    d = 3.14;
    return sizeof(d);
}
