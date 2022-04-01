int main()
{
    int x = 99;
    int y = 10;
    int iter = 0;
    while (x > 0)
    {
        x = x - 1;
        iter = iter + 1;
        while (y > 0)
        {
            y = y - 10;
        }
        y = 10;
    }
    return iter;
}
