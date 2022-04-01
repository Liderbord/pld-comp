int hello(int b, int c, int d, int e, int f, int g) {
    int a = e + 4 * f + g - 33;
    return a;
}

int main() {
    int x = hello(1, 2, 3, 4, 5, 6) + hello(6, 10, 3, 4, 5, 6);
    int y = hello(1, 2, 3, 4, 5, 11) * hello(6, 10, 55, 4, 11, 6);
    return y;
}
