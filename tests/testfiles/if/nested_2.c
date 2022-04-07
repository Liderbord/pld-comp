int main()
{
    int test = 3;
    if (1)
    {
        test = 1;
        if (0)
        {
            test = 0;
        }
    }
    return test;
}