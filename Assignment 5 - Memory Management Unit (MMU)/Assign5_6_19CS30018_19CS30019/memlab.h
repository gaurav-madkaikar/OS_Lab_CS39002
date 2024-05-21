// Group 6
// OS Lab Assignment 5 - Memory Management
// Gaurav Madkaikar (19CS30018)
// Girish Kumar     (19CS30019)

#ifndef MEMLAB_H
#define MEMLAB_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_FREE_SPACES 100

// Garbage Collector Thread
pthread_t gc_thread;

// Mutex lock
pthread_mutex_t lock;

// Signal to break out of the thread loop
bool breakSignal;

// Signal to rmeove entries from the global stack
bool flushGarbage;

int validValueCheck(char *dataType, int value);
void *gc_run(void *args);
void gc_initialize();

// Static array for holding freed memory spaces
typedef struct list
{
    void *phyAddress;
    int size;
    int valid;
} array;
array freeMemList[MAX_FREE_SPACES];


void initFreeMemList();
void addFreeMemList(void *phyAddr, int size);

// Linked List for pageTable
typedef struct PTnode
{
    int localAddress;       // Local address (counter value * 4)
    char *name;             // Name of the variable
    char *type;             // Data type of the variable
    unsigned int size;      // Alocated size
    int functionScope;      // Value of function counter
    void *phyAddress;       // Address in physical memory
    bool listType;          // Check if the entry is a list or not
    bool valid;             // Check if it is used/unused
    struct PTnode *next;    // Next pointer 
} listNode;

listNode *pageTable, *currPageTable;
void *pageTablePtr;

// Create a new pageTable entry
listNode *createListNode(char *varName, char *type, size_t size, int listType);

// Insert a new entry into the pageTable
void insertInList(listNode *nptr);

// Delete an entry from the pageTable
void deleteInList(char *varName);

// Stack Structure
typedef struct node
{
    char *varName;
    void *phyAddress;
    int allocatedSize;
    int functionCntr;
    struct node *next;
} stackNode;

typedef struct stack
{
    int size;
    stackNode *head;
} stack;
stack globalStack, pageTableStack;

// Stack functions
void initStack(stack *startPtr);
stackNode *createStackNode(char *varName, void *addr, int allocatedSize);
void push(char *varName, stack *stackPtr, void *addr, int allocatedSize);
int pop(stack *stackPtr);
void display();

// Extract physical address
void *genPhyAddress(char *varName);
// Extract data type
char *genDataType(char *varName);
void startFunction();
void endFunction();

// Base pointer, Pointer to the page table, Pointer to the physical address space
void *basePtr, *phyPtr, *currphyPtr;

// Allocate specified amount of memory
void createMem(size_t size);

// Create a variable of specified type
void createVar(char *varName, char *type);

// Assign value to variable
void assignVar(char *varName, int value);

// Create array of specified "type"
void createArr(char *varName, char *type, size_t size);

// Assign values to array elements
void assignArr(char *varName, int index, int value);

// Free the allocated memory
void freeMem();

// Free unused elements
void freeElem();

#endif