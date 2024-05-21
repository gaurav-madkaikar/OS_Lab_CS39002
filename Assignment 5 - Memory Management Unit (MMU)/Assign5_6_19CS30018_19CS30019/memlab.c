// Group 6
// OS Lab Assignment 5 - Memory Management
// Gaurav Madkaikar (19CS30018)
// Girish Kumar     (19CS30019)

// ------------------- START -------------------

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include "memlab.h"

// Maximum of 100 entries in the pageTable
int maxPTSize = sizeof(listNode) * 100, currPTSize = 0;
int freeMemListSize = 0, functionCntr = 0;


// freeMemList Functions
// Initialize freeMemList
void initFreeMemList()
{
    for (int i = 0; i < MAX_FREE_SPACES; i++)
    {
        freeMemList[i].phyAddress = NULL;
        freeMemList[i].valid = 0;
        freeMemList[i].size = 0;
    }
    printf("\n[+] Initialize freeMemList!\n");
    return;
}

// Insert a free physical address into the freeMemList
void addFreeMemList(void *phyAddr, int size)
{
    for (int i = 0; i < MAX_FREE_SPACES; i++)
    {
        if (freeMemList[i].valid == 0)
        {
            freeMemList[i].phyAddress = phyAddr;
            freeMemList[i].size = size;
            freeMemList[i].valid = 1;
            freeMemListSize++;
            printf("\n[+] Free physical address added into the freeMemList!\n");
            return;
        }
    }
    return;
}

// pageTable Functions
// Add a new entry into the pageTable
listNode *createListNode(char *varName, char *type, size_t size, int listType)
{
    listNode *newNode;

    // Allocate address in logical memory
    // If there is an available address in the stack, allocate that to the new entry of the pageTable
    if (pageTableStack.size > 0)
    {
        newNode = (listNode *)pageTableStack.head->phyAddress;
        pop(&pageTableStack);
    }
    // If no free memory available, increment the pointer
    else
    {
        newNode = (listNode *)pageTablePtr;
        pageTablePtr += sizeof(listNode);
    }
    newNode->localAddress = 4 * (currPTSize++);
    newNode->name = varName;
    newNode->valid = true;
    newNode->type = type;
    newNode->size = size;
    newNode->listType = listType;
    newNode->functionScope = functionCntr;
    newNode->next = NULL;

    // Allocate address in physical memory
    // If there is an available address in the stack, allocate that to the new entry of the physical memory
    int flg = 0;
    if (freeMemListSize > 0)
    {
        void *assignAddress = NULL;
        for (int i = 0; i < MAX_FREE_SPACES; i++)
        {
            if ((freeMemList[i].valid == 1) && (newNode->size <= freeMemList[i].size))
            {
                assignAddress = freeMemList[i].phyAddress;

                // Mark the free space as occupied / invalid
                freeMemList[i].phyAddress = NULL;
                freeMemList[i].size = 0;
                freeMemList[i].valid = 0;
                // Decrement size of the freeMemList
                freeMemListSize--;

                flg = 1;
                newNode->phyAddress = assignAddress;
                break;
            }
        }
    }
    // If no free memory available, increment the pointer
    if (!flg)
    {
        newNode->phyAddress = currphyPtr;
        if (listType == 0)
            currphyPtr += 4;
        else
            currphyPtr += size;
    }
    printf("[+] New pageTable entry initialized!\n");
    return newNode;
}

void insertInList(listNode *nptr)
{
    if (pageTable == NULL)
    {
        pageTable = nptr;
        currPageTable = nptr;
    }
    else
    {
        currPageTable->next = nptr;
        currPageTable = nptr;
    }
    // Increment size of the pageTable
    currPTSize++;

    printf("[+] New pageTable entry inserted!\n");
    return;
}

void deleteInList(char *varName)
{
    int cnt = 1;
    listNode *temp, *prev;
    temp = pageTable;
    prev = pageTable;

    if (temp == NULL)
    {
        printf("[-] ERROR: Page Table is empty!\n");
        return;
    }
    while ((strcmp(temp->name, varName)) && (temp != NULL))
    {
        cnt++;
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL)
    {
        printf("[-] ERROR: Requested pageTable entry not available!\n");
        return;
    }
    else
    {
        // Deletion of the first element
        if (cnt == 1) 
        {
            if (temp == currPageTable)
            {
                currPageTable = temp->next;
            }
            pageTable = temp->next;
            free(temp);
        }
        // Deletion of elements at other positions
        else 
        {
            if (temp == currPageTable)
            {
                currPageTable = prev;
            }
            prev->next = temp->next;
            free(temp);
        }
        // Decrement size of the pageTable
        currPTSize--;
        printf("[+] New pageTable entry removed!\n");
    }
}

// Stack functions
void initStack(stack *stackPtr)
{
    stackPtr->head = NULL;
    stackPtr->size = 0;
    printf("[+] Stack initialized successfully!\n");
}

// Create stack node
stackNode *createStackNode(char *varName, void *addr, int allocatedSize)
{
    stackNode *newNode = malloc(sizeof(stackNode));
    newNode->varName = varName;
    newNode->phyAddress = addr;
    newNode->allocatedSize = allocatedSize;
    newNode->functionCntr = functionCntr;
    newNode->next = NULL;
    printf("[+] Stack initialized successfully!\n");
    return newNode;
}

// Push stack element
void push(char *varName, stack *stackPtr, void *addr, int allocatedSize)
{
    // Create a new node
    stackNode *newNode = createStackNode(varName, addr, allocatedSize);
    newNode->next = stackPtr->head;
    stackPtr->head = newNode;
    stackPtr->size++;
    printf("[+] Stack entry pushed successfully!\n");
}

// Pop stack element
int pop(stack *stackPtr)
{
    // temp is used to free the head node
    stackNode *temp;
    if (stackPtr->head == NULL)
    {
        printf("[+] Empty stack!\n");
        return 0;
    }
    else
    {
        printf("[+] Popped element at physical address = %p\n", (stackPtr->head)->phyAddress);

        // backup the head node
        temp = stackPtr->head;

        // Make the head node points to the next node.
        stackPtr->head = stackPtr->head->next;

        // Decrement the size of the stack
        stackPtr->size--;

        // Free the popped element's memory
        free(temp);
    }
    return 1;
}

// Check if the assigned value fits the size of the corresponding data type
int validValueCheck(char *dataType, int value)
{
    if (!strcmp(dataType, "medium int"))
    {
        int upp_limit = (1LL << 24) - 1;
        int low_limit = -(1LL << 24);
        if ((value > upp_limit) || (value < low_limit))
        {
            return 0;
        }
    }
    else if (!strcmp(dataType, "char"))
    {
        if ((value < -128) && (value > 127))
        {
            return 0;
        }
    }
    else if (!strcmp(dataType, "bool"))
    {
        if ((value != 0) && (value != 1))
        {
            return 0;
        }
    }

    return 1;
}

void *gc_run(void *args)
{
    printf("[+] gc_run...\n");
    time_t sleep_time;
    while (1)
    {
        if (breakSignal == 1)
        {
            break;
        }
        // Randomise seed
        srand(time(0));
        listNode *startPtr = pageTable;

        pthread_mutex_lock(&lock);
        // Critical section
        if (flushGarbage)
        {
            stackNode *tmpNode = globalStack.head;
            char *varName = NULL;
            // Free unused memory
            while ((tmpNode != NULL) && (tmpNode->functionCntr == functionCntr))
            {
                varName = tmpNode->varName;
                // Add the physical memory location to freeMemList
                addFreeMemList(tmpNode->phyAddress, tmpNode->allocatedSize);

                // Free the allocated physical memory
                void *tmpAddr = tmpNode->phyAddress;
                for (int i = 0; i < ceil((float)tmpNode->allocatedSize / 4); i++)
                {
                    int *tmpPtr = (int *)tmpAddr;
                    tmpPtr = NULL;
                }

                // Pop the variable from the global stack
                int popStatus = pop(&globalStack);
                if (popStatus)
                {
                    printf("Popped variable name: %s\n\n", varName);
                }

                tmpNode = tmpNode->next;
            }
            // Free the marked elements in the pageTable
            listNode *tempPtr = pageTable;
            while (tempPtr != NULL)
            {
                if (tempPtr->valid == false)
                {
                    char *delVar = tempPtr->name;
                    deleteInList(delVar);
                    tempPtr = pageTable;
                    continue;
                }
                tempPtr = tempPtr->next;
            }
            // Decrement local function scope
            functionCntr--;
            printf("[+] Garbage successfully flushed out!\n");
        }
        pthread_mutex_unlock(&lock);

        // Sleep for a random time of 1-3 sec
        sleep_time = (rand() % 3) + 1;
        sleep(sleep_time);
    }
    pthread_exit(0);
    return NULL;
}

// Initialize gc_thread
void gc_initialize()
{
    pthread_create(&gc_thread, NULL, gc_run, NULL);
    return;
}

// Allocate "size" bytes of memory (physical memory)
void createMem(size_t size)
{
    // Allocate the required size for storing the pageTable and the physical memory
    basePtr = malloc(size);
    pageTablePtr = basePtr;

    // Allocate space for the pageTable
    phyPtr = basePtr + maxPTSize;
    currphyPtr = phyPtr;

    printf("[+] Required size of memory allocated!\n\n");

    // Initialise the stacks
    initStack(&globalStack);
    initStack(&pageTableStack);

    // Initialise the freeMem list
    initFreeMemList();

    // Initialise the pageTable pointers
    pageTable = NULL;
    currPageTable = NULL;

    // Run the garbage collection thread
    breakSignal = false;
    flushGarbage = false;
    // gc_initialize();

    return;
}

// Extract data type
char *genDataType(char *varName)
{
    char *type = NULL;
    listNode *startPtr = pageTable;
    while (startPtr != NULL)
    {
        if (strcmp(varName, startPtr->name) == 0)
        {
            type = startPtr->type;
            break;
        }
        startPtr = startPtr->next;
    }
    return type;
}

// Extract physical address
void *genPhyAddress(char *varName)
{
    void *phyAddr = NULL;
    listNode *startPtr = pageTable;
    while (startPtr != NULL)
    {
        if (strcmp(varName, startPtr->name) == 0)
        {
            phyAddr = startPtr->phyAddress;
            break;
        }
        startPtr = startPtr->next;
    }
    return phyAddr;
}

// Create a variable
void createVar(char *varName, char *type)
{
    if (!strcmp(type, "int"))
    {
        listNode *newNode = createListNode(varName, type, sizeof(int), 0);
        insertInList(newNode);

        // Add to the global stack
        push(varName, &globalStack, newNode->phyAddress, newNode->size);

        printf("[+] Integer variable %s created!\n", currPageTable->name);
    }
    else if (!strcmp(type, "char"))
    {
        listNode *newNode = createListNode(varName, type, sizeof(char), 0);
        insertInList(newNode);

        // Add to the global stack
        push(varName, &globalStack, newNode->phyAddress, newNode->size);

        printf("[+] Character variable %s created!\n", currPageTable->name);
    }
    else if (!strcmp(type, "bool"))
    {
        listNode *newNode = createListNode(varName, type, sizeof(bool), 0);
        insertInList(newNode);

        // Add to the global stack
        push(varName, &globalStack, newNode->phyAddress, newNode->size);

        printf("[+] Boolean variable %s created!\n", currPageTable->name);
    }
    else if (!strcmp(type, "medium int"))
    {
        listNode *newNode = createListNode(varName, type, 3, 0);
        insertInList(newNode);

        // Add to the global stack
        push(varName, &globalStack, newNode->phyAddress, newNode->size);

        printf("[+] Medium integer variable %s created!\n", currPageTable->name);
    }
    else
    {
        printf("[-] ERROR: Invalid data type!\n");
        freeMem();
        exit(EXIT_FAILURE);
    }
    return;
}

// Assign value to variable
void assignVar(char *varName, int value)
{
    int recvVal = value;
    char *varType = NULL;
    void *phyAddr = NULL;
    listNode *startPtr = pageTable;

    while (startPtr != NULL)
    {
        if (strcmp(startPtr->name, varName) == 0)
        {
            varType = startPtr->type;
            phyAddr = startPtr->phyAddress;
            break;
        }
        startPtr = startPtr->next;
    }
    if (varType == NULL)
    {
        printf("[-] ERROR: Invalid variable name!\n");
        return;
    }
    printf("[+] Variable %s = ", varName);
    if (!strcmp(varType, "medium int"))
    {
        // Type checking
        if (!validValueCheck(varType, recvVal))
        {
            printf("[-] ERROR: Invalid value assigned!\n");
            return;
        }
        // Assign values in the pageTable and the physical memory
        int *tmpVal = (int *)phyAddr;
        *tmpVal = recvVal;

        // Print the variable value
        printf("%d\n", *tmpVal);
    }
    else if (!strcmp(varType, "int"))
    {
        // Assign values in the pageTable and the physical memory
        int *tmpVal = (int *)phyAddr;
        *tmpVal = recvVal;
        printf("%d\n", *tmpVal);
    }
    else if (!strcmp(varType, "char"))
    {
        // Type checking
        if (!validValueCheck(varType, recvVal))
        {
            printf("[-] ERROR: Invalid value assigned!\n");
            return;
        }
        // Assign values in the pageTable and the physical memory
        char *tmpVal = (char *)phyAddr;
        *tmpVal = recvVal;
        printf("%c\n", *tmpVal);
    }
    else if (!strcmp(varType, "bool"))
    {
        // Type checking
        if (!validValueCheck(varType, recvVal))
        {
            printf("[-] ERROR: Invalid value assigned!\n");
            return;
        }
        // Assign values in the pageTable and the physical memory
        bool *tmpVal = (bool *)phyAddr;
        *tmpVal = recvVal;
        printf("%s\n", *tmpVal ? "true" : "false");
    }
}

// Create a new array
void createArr(char *varName, char *type, size_t size)
{
    // int numBlocks = ceil((float)size / 4);
    int numBlocks = (float)size/4;

    if (!strcmp(type, "int"))
    {
        listNode *newNode = createListNode(varName, type, sizeof(int) * size, 1);
        insertInList(newNode);

        // Add to the global stack
        push(varName, &globalStack, newNode->phyAddress, newNode->size);

        // Increment physical memory pointer
        currphyPtr += 4 * size;
    }
    else if (!strcmp(type, "char"))
    {
        listNode *newNode = createListNode(varName, type, sizeof(char) * size, 1);
        insertInList(newNode);

        // Add to the global stack
        push(varName, &globalStack, newNode->phyAddress, newNode->size);

        // Increment physical memory pointer
        currphyPtr += 4 * numBlocks;
    }
    else if (!strcmp(type, "bool"))
    {
        listNode *newNode = createListNode(varName, type, sizeof(bool) * size, 1);
        insertInList(newNode);

        // Add to the global stack
        push(varName, &globalStack, newNode->phyAddress, newNode->size);

        // Increment physical memory pointer
        currphyPtr += 4 * numBlocks;
    }
    else if (!strcmp(type, "medium int"))
    {
        listNode *newNode = createListNode(varName, type, 3 * size, 1);
        insertInList(newNode);

        // Add to the global stack
        push(varName, &globalStack, newNode->phyAddress, newNode->size);

        // Increment physical memory pointer
        currphyPtr += 4 * size;
    }
    else
    {
        printf("[-] ERROR: Invalid data type!\n");
        freeMem();
        exit(EXIT_FAILURE);
    }
    return;
}

// Assign values to array elements
void assignArr(char *varName, int index, int value)
{
    void *arrPtr = genPhyAddress(varName);
    char *varType = genDataType(varName);
    if (varType == NULL)
    {
        printf("[+] ERROR: Invalid variable name!\n");
        return;
    }
    else if (!strcmp(varType, "int"))
    {
        int *tmpPtr = (int *)arrPtr;
        *tmpPtr = value;
        printf("%d ", *(int *)arrPtr);
    }
    else if (!strcmp(varType, "char"))
    {
        char *tmpPtr = (char *)arrPtr;
        *tmpPtr = (char)value;
        printf("%c ", *(char *)arrPtr);
    }
    else if (!strcmp(varType, "bool"))
    {
        bool *tmpPtr = (bool *)arrPtr;
        *tmpPtr = (bool)value;
        printf("%s ", *(bool *)arrPtr ? "true" : "false");
    }
    else if (!strcmp(varType, "medium int"))
    {
        if (!validValueCheck(varType, value))
        {
            printf("[-] ERROR: Invalid value assigned!\n");
            return;
        }
        int *tmpPtr = (int *)arrPtr;
        *tmpPtr = value;
        printf("%d ", *(int *)arrPtr);
    }
}

// Free unused elements (Call at the end of function body)
void freeElem()
{
    listNode *startPtr = pageTable;

    // Mark variables to be removed in the Internal Page Table
    while ((startPtr != NULL))
    {
        // If variable found in local scope, mark as invalid
        if (startPtr->functionScope == functionCntr)
        {
            startPtr->valid = false;
        }
        startPtr = startPtr->next;
    }

    // Free unused variables in the current scope
    flushGarbage = true;

    printf("[+] Elements to be freed marked successfully!\n");

    return;
}

// Initialize scope of a function
void startFunction()
{
    printf("[+] New function starts here...\n\n");
    // New local scope defined
    functionCntr++;
    return;
}


// Indicate end of a function
void endFunction()
{
    // Free used elements
    freeElem();
    printf("[+] Function ends here...\n\n");
}

// Free the total memory allocated
void freeMem()
{
    // Finish execution of the thread
    breakSignal = true;

    // Wait till the thread exits
    // pthread_join(gc_thread, NULL);

    // Free the allocated memory
    free(basePtr);

    printf("\n[+] Allocated chunk of memory freed successfully!\nExiting program...\n");

    return;
}
// ------------------- END -------------------

// ---------------------- Code to test our library ----------------------

// void functr()
// {
//     startFunction();
//     createVar("hel", "int");
//     assignVar("hel", 5);
//     endFunction();
//     return;
// }

// int main(int argc, char *argv[])
// {
//     time_t start = clock(), end;
//     startFunction();

//     createMem(1000);
//     printf("%p, %p\n", pageTablePtr, phyPtr);
//     createVar("gau", "int");
//     createVar("tmp", "char");
//     createVar("tmp2", "bool");
//     assignVar("gau", 4);
//     assignVar("tmp", 'b');
//     assignVar("tmp2", true);

//     functr();

//     createArr("arr", "int", 4);
//     int tmpArr[] = {10, 12, 37, 54};
//     for (int i = 0; i < 4; i++)
//     {
//         assignArr("arr", i, tmpArr[i]);
//     }
//     printf("\n");
//     createArr("arr2", "char", 4);
//     char newtmp[] = {'a', 'b', 'c', 'd'};
//     for (int i = 0; i < 4; i++)
//     {
//         assignArr("arr2", i, newtmp[i]);
//     }
//     printf("\n");

//     endFunction();

//     freeMem();
//     return 0;
// }