#include <stdio.h>
#include <stdlib.h>
#include "memlab.h"

#define MEM_SIZE 100000

int fibonacci(int n)
{
    startFunction();
    if (n == 1 | n == 0)
        return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);

    endFunction();
    return 0;
}

int fibonacciProduct(int k)
{
    startFunction();
    createArr("arr", "int", k);
    int ans = 1;
    int val = 1;
    for (int i = 0; i < k; i++)
    {
        val = fibonacci(i);
        assignArr("arr", i, val);
        ans *= val;
    }
    endFunction();
    return ans;
}

int main()
{
    printf("\n [+] demo2.c !\n\n");

    createMem(MEM_SIZE);
    int k;
    scanf("%d", &k);
    int prod = fibonacciProduct(k);

    printf("\nProduct is : %d\n", k);
    printf("\n [+] demo2.c ends here!\n\n");
    return 0;
}
