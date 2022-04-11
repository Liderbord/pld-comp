int main() {
    int result;
    result = recursion(5);
    return result;
}

int recursion(int n) {
    if (n != 0)
        return n + recursion(n-1); 
    else
        return n;
}