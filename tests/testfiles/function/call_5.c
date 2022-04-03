int hello() {
    return 1;
}

int main() {
    int x = hello() + hello();
    return x;
}
