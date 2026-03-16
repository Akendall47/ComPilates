int f(int n);

int main()
{
    /* f(3) = 3+5 = 8, f(-2) = 5-(-2) = 7 */
    return !(f(3) == 8 && f(-2) == 7);
}
