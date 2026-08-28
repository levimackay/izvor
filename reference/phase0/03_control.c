/* Reference solution — task 0.3 */

int max3(int a, int b, int c) {
    int m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    return m;
}

long factorial(int n) {
    long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

int gcd(int a, int b) {
    while (b != 0) {
        int r = a % b;
        a = b;
        b = r;
    }
    return a;
}

int count_digits(int n) {
    int count = 1;               /* 0 has one digit */
    while (n >= 10) {
        n /= 10;
        count++;
    }
    return count;
}
