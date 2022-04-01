int main()
{
    // note: did not make a while(1) tests because I do not want an infinite loop
    int x = 99;
    int iter = 0;
    while (x == 0)
    {
        x = x - 1;
        iter = iter + 1;
    }
    return iter;
}
