#include <stdio.h>

double fib(double i);
double fibi(double i);

int main()
{
    for (int i = 1; i < 10; i++) {
        printf("fib(%d) = %d\n", i, (int)fib(i));
    }
    for (int i = 1; i < 10; i++) {
        printf("fibi(%d) = %d\n", i, (int)fibi(i));
    }
    return 0;
}