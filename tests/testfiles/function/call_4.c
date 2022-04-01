int hello(int b, int c, int d, int e, int f, int g) {
    int a = e + 4 * f + g - 33;
    int y = 11;
    return a + y;
}

int bye() {
    int x = hello(1, 2, 3, 4, 5, 6);
    return x;
}


int main() {
    int x = hello(1, 2, 3, 4, 5, 6);
    return x;
}
