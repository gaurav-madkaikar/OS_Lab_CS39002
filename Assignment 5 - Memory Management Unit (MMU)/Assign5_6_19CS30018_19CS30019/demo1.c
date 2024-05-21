#include <stdio.h>
#include "memlab.h"

// Required Memory: 250 * 10^6 bytes
#define MAX_REQ_MEM 250000000

void func1(int x, int y)
{
    startFunction();
    createVar("localVar1", "int");
    createVar("localVar2", "int");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "int", 50000);
    for (int i = 0; i < 50000; i++)
    {
        int val = (rand() % 31) - 30;
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func2(char x, char y)
{
    startFunction();
    createVar("localVar1", "char");
    createVar("localVar2", "char");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "char", 50000);
    for (int i = 0; i < 50000; i++)
    {
        char val = (char)((rand() % 26) + (rand() % 2 ? 'a' : 'A'));
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func3(bool x, bool y)
{
    startFunction();
    createVar("localVar1", "bool");
    createVar("localVar2", "bool");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "bool", 50000);
    for (int i = 0; i < 50000; i++)
    {
        bool val = (bool)(rand() % 2);
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func4(int x, int y)
{
    startFunction();
    createVar("localVar1", "medium int");
    createVar("localVar2", "medium int");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "medium int", 50000);
    for (int i = 0; i < 50000; i++)
    {
        int val = (rand() % 100) - 99;
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func5(bool x, bool y)
{
    startFunction();
    createVar("localVar1", "bool");
    createVar("localVar2", "bool");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "bool", 50000);
    for (int i = 0; i < 50000; i++)
    {
        bool val = (bool)(rand() % 2);
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func6(char x, char y)
{
    startFunction();
    createVar("localVar1", "char");
    createVar("localVar2", "char");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "char", 50000);
    for (int i = 0; i < 50000; i++)
    {
        char val = (char)((rand() % 26) + (rand() % 2 ? 'a' : 'A'));
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func7(int x, int y)
{
    startFunction();
    createVar("localVar1", "medium int");
    createVar("localVar2", "medium int");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "medium int", 50000);
    for (int i = 0; i < 50000; i++)
    {
        int val = (rand() % 10000) - 10000;
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func8(int x, int y)
{
    startFunction();
    createVar("localVar1", "int");
    createVar("localVar2", "int");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "int", 50000);
    for (int i = 0; i < 50000; i++)
    {
        int val = (rand() % 4000) - 4000;
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func9(char x, char y)
{
    startFunction();
    createVar("localVar1", "char");
    createVar("localVar2", "char");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "char", 50000);
    for (int i = 0; i < 50000; i++)
    {
        char val = (char)((rand() % 26) + (rand() % 2 ? 'a' : 'A'));
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

void func10(int x, int y)
{
    startFunction();
    createVar("localVar1", "medium int");
    createVar("localVar2", "medium int");

    assignVar("localVar1", x);
    assignVar("localVar2", y);

    createArr("arr", "medium int", 50000);
    for (int i = 0; i < 50000; i++)
    {
        int val = (rand() % 17000) - 16999;
        assignArr("arr", i, val);
    }
    printf("\n");
    endFunction();
}

// demo1.c
int main()
{
    printf("[+] demo1.c !\n\n");
    size_t requiredSize = MAX_REQ_MEM;
    createMem(requiredSize);

    createVar("int1", "int");
    createVar("int2", "int");

    createVar("char1", "char");
    createVar("char2", "char");

    createVar("mediumInt1", "medium int");
    createVar("mediumInt2", "medium int");

    createVar("Bool1", "bool");
    createVar("Bool2", "bool");

    func1(4, 5);
    func2(-2, 3);
    func3(true, false);
    func4(-1, 0);
    func5(false, false);
    func6('b','z');
    func7(65534, -65534);
    func8(-1324555, -9009);
    func9('.','A');
    func10(-91, 4257);
    return 0;
}