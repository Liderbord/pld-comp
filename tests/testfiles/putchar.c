#include<stdio.h>

int hello() {}

int main() {
    int a = 0;
    int cond = a <= 3;
    while (cond) {
        putchar('a');
        a = a + 1;
        cond = a <= 3;
    }
    return 0;
}