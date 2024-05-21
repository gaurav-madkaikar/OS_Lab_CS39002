#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
using namespace std;

#define MAX_QUEUE_SIZE 8
#define MATRIX_SIZE 1000
#define BLOCK_SIZE 500
#define MAXPRIME (int)(1e6 + 7)
#define TERMINATE 8

typedef struct Job
{
    int prodNum;
    int statusDone;
    double matrix[MATRIX_SIZE][MATRIX_SIZE];
    int matId;
    int workId;
} job;

typedef struct Queue
{
    job jobList[MAX_QUEUE_SIZE];
    int maxSize, currSize;
    int front, rear;
    int progress; // True if the queue is currently undergoing any operation
    int workId;
}pqueue;

// Insert and remove jobs
int insertJob(pqueue *ptr, job J, int opType)
{
    // Queue is full
    if (ptr->currSize >= MAX_QUEUE_SIZE)
    {
        return 0;
    }
    if (ptr->rear == MAX_QUEUE_SIZE)
    {
        for (int i = ptr->front; i < ptr->rear; i++)
            ptr->jobList[i - 1] = ptr->jobList[i];

        ptr->rear--;
        ptr->front--;
    }
    ptr->jobList[ptr->rear] = J;

    ptr->rear++;
    ptr->currSize++;

    return 1;
}

int removeJob(pqueue *ptr)
{
    if (ptr->currSize == 0)
    {
        return 0;
    }
    ptr->front++;
    ptr->currSize--;
    if (ptr->currSize == 0)
    {
        ptr->front = 0;
        ptr->rear = 0;
    }
    return 1;
}

// Create job with random values assigned to their variables
job createJob(int prodNum, int opType = 0)
{
    job newJob;
    int matId = (rand() % 100000) + 1;
    int val = (rand() % 19) - 9;
    if (opType)
        val = 0;

    newJob.prodNum = prodNum;
    newJob.statusDone = 0;
    newJob.matId = matId;
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            newJob.matrix[i][j] = val;
        }
    }
    return newJob;
}

int main(int argc, char *argv[])
{
    // Initialise the time seed
    srand(time(0));
    clock_t start, end;

    int NP, NW, numMat;
    printf("Number of producers: ");
    scanf("%d", &NP);
    printf("\nNumber of workers: ");
    scanf("%d", &NW);

    printf("\nEnter the number of matrices to be multiplied: ");
    scanf("%d", &numMat);

    start = clock();

    // Allocate shared memory
    int reqMemory = sizeof(pqueue) + sizeof(int) + (MAX_QUEUE_SIZE * sizeof(job));
    int shmid = shmget(30, reqMemory, 0644 | IPC_CREAT);
    char *memSegment;
    if (shmid == -1)
    {
        perror("Failed to allocate shared memory!");
        exit(EXIT_FAILURE);
    }

    // Attach all variables to the shared memory segment
    memSegment = (char *)shmat(shmid, NULL, 0);
    pqueue *ptr = (pqueue *)memSegment;
    int *jobs_created = (int *)(memSegment + sizeof(pqueue));

    // Initialise variable values
    ptr->maxSize = numMat;
    ptr->currSize = 0;
    ptr->front = ptr->rear = 0;
    ptr->progress = 0;
    *jobs_created = 0;

    pid_t pid1, pid2;

    // Create producer processes
    for (int prod = 0; prod < NP; prod++)
    {
        // Fork processes
        pid1 = fork();
        if (pid1 < 0)
        {
            perror("Process cannot be forked!");
            exit(EXIT_FAILURE);
        }
        else if (pid1 == 0)
        {
            // Re-initialise seed
            srand((time(NULL) * (prod + 1)) % MAXPRIME);

            // Producer ID (ID of the child process)
            pid_t prodId = getpid();

            int insertStatus = 0, flg = 0;

            while (*jobs_created < numMat)
            {
                // Generate a job
                job newJob = createJob(prod + 1);
                // Keep waiting until there is enough space in queue
                while (!insertStatus)
                {
                    // Wait for 0-3 s
                    sleep(rand() % 4);

                    if (*jobs_created >= numMat)
                    {
                        flg = 1;
                        break;
                    }

                    if (ptr->progress == 1)
                        continue;

                    // Queue is marked busy
                    ptr->progress = 1;
                    // Insert job into the queue
                    insertStatus = insertJob(ptr, newJob, 0);
                    // Queue is now available
                    ptr->progress = 0;
                }
                if (flg)
                    break;
                *jobs_created++;
                printf("Producer Job Creation Task\nProducer ID: %d\nProcess Number: %d\n Status: %d\n\n", prodId, prod + 1, newJob.statusDone);
            }
            // Detach memory segment and exit
            shmdt(memSegment);
            exit(EXIT_SUCCESS);
        }
    }
    double ptr1[MATRIX_SIZE][MATRIX_SIZE], ptr2[MATRIX_SIZE][MATRIX_SIZE];
    job JOB;
    for (int work = 0; work < NW; work++)
    {
        double D1[500][500], D2[500][500], D3[500][500], D4[500][500], D5[500][500], D6[500][500], D7[500][500], D8[500][500];
        pid2 = fork();
        if (pid2 == -1)
        {
            perror("Process cannot be forked!");
            exit(EXIT_FAILURE);
        }
        else if (pid2 == 0)
        {
            // Re-initialise seed
            srand(time(NULL) ^ (work + 1));

            // While all jobs are not complete
            while (*jobs_created < numMat || ptr->currSize != 1)
            {
                job job1, job2;
                // Minimum 2 jobs are required
                if (ptr->currSize <= 1)
                {
                    continue;
                }

                // Wait for 0-3 s
                sleep(rand() % 4);

                JOB = ptr->jobList[ptr->front];

                // Retrieve current status
                int currentStatus = ptr->jobList[ptr->front].statusDone;
                if (currentStatus == TERMINATE)
                    continue;

                // Retrieve the addresses of the 2 blocks
                memcpy(ptr1, ptr->jobList[ptr->front].matrix, sizeof(ptr1));
                memcpy(ptr2, ptr->jobList[ptr->front + 1].matrix, sizeof(ptr2));

                double **Cij;
                // Worker processes for multiplication of blocks
                if (currentStatus == 0)
                {
                    job newJob = createJob(-1, 1);
                    newJob.workId = work;
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            D1[i][j] = 0;
                            for (int k = 0; k < BLOCK_SIZE; k++)
                                D1[i][j] += ptr1[i][k] * ptr2[k][j];
                        }
                    }
                    newJob.statusDone = 1;
                    if (newJob.workId == ptr->jobList[ptr->front].workId)
                    {
                        for (int i = 0; i < BLOCK_SIZE; i++)
                        {
                            for (int j = 0; j < BLOCK_SIZE; j++)
                            {
                                newJob.matrix[i][j] = D1[i][j];
                            }
                        }
                    }
                    int insertStatus = 1;
                    // Add job to the queue
                    while (!insertStatus)
                    {
                        // Wait for 0-3 s
                        sleep(rand() % 4);

                        if (ptr->progress == 1)
                        {
                            continue;
                        }
                        // Queue is marked busy
                        ptr->progress = 1;
                        // Insert job into the queue
                        insertStatus = insertJob(ptr, newJob, 0);
                        // Queue is now available
                        ptr->progress = 0;
                    }
                    printf("Worker Process\nMatrix ID: %d\nWork Number: %d\n Status: %d\n\n", ptr->jobList[ptr->front].matId, work + 1, newJob.statusDone);
                }
                else if (currentStatus == 1)
                {
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            D2[i][j] = 0;
                            for (int k = 0; k < BLOCK_SIZE; k++)
                                D2[i][j] += ptr1[i][k+BLOCK_SIZE] * ptr2[k+BLOCK_SIZE][j];
                        }
                    }
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            ptr->jobList[ptr->rear].matrix[i][j] += D2[i][j];
                        }
                    }
                    ptr->jobList[ptr->rear].statusDone = 2;
                    printf("Worker Process\nMatrix ID: %d\nWork Number: %d\n Status: %d\n\n", ptr->jobList[ptr->rear].matId, work + 1, ptr->jobList[ptr->rear].statusDone);
                }
                else if (currentStatus == 2)
                {
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            D3[i][j] = 0;
                            for (int k = 0; k < BLOCK_SIZE; k++)
                                D3[i][j] += ptr1[i][k] * ptr2[k][j+BLOCK_SIZE];
                        }
                    }
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            ptr->jobList[ptr->rear].matrix[i+BLOCK_SIZE][j] = D3[i][j];
                        }
                    }
                    ptr->jobList[ptr->rear].statusDone = 3;
                    printf("Worker Process\nMatrix ID: %d\nWork Number: %d\n Status: %d\n\n", ptr->jobList[ptr->rear].matId, work + 1, ptr->jobList[ptr->rear].statusDone);
                }
                else if (currentStatus == 3)
                {
                    for (int i = BLOCK_SIZE; i < MATRIX_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            D3[i][j] = 0;
                            for (int k = 0; k < BLOCK_SIZE; k++)
                                D3[i][j] += ptr1[i][k+BLOCK_SIZE] * ptr2[k+BLOCK_SIZE][j+BLOCK_SIZE];
                        }
                    }
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            ptr->jobList[ptr->rear].matrix[i+BLOCK_SIZE][j] += D4[i][j];
                        }
                    }
                    ptr->jobList[ptr->rear].statusDone = 4;
                    printf("Worker Process\nMatrix ID: %d\nWork Number: %d\n Status: %d\n\n", ptr->jobList[ptr->rear].matId, work + 1, ptr->jobList[ptr->rear].statusDone);
                }
                else if (currentStatus == 4)
                {
                    for (int i = BLOCK_SIZE; i < MATRIX_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            D3[i][j] = 0;
                            for (int k = 0; k < BLOCK_SIZE; k++)
                                D3[i][j] += ptr1[i][k+BLOCK_SIZE] * ptr2[k+BLOCK_SIZE][j+BLOCK_SIZE];
                        }
                    }
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            ptr->jobList[ptr->rear].matrix[i+BLOCK_SIZE][j] += D2[i][j];
                        }
                    }
                    ptr->jobList[ptr->rear].statusDone = 5;
                    printf("Worker Process\nMatrix ID: %d\nWork Number: %d\n Status: %d\n\n", ptr->jobList[ptr->rear].matId, work + 1, ptr->jobList[ptr->rear].statusDone);
                }
                else if (currentStatus == 5)
                {
                    for (int i = BLOCK_SIZE; i < MATRIX_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            D3[i][j] = 0;
                            for (int k = 0; k < BLOCK_SIZE; k++)
                                D3[i][j] += ptr1[i][k+BLOCK_SIZE] * ptr2[k+BLOCK_SIZE][j+BLOCK_SIZE];
                        }
                    }
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            ptr->jobList[ptr->rear].matrix[i+BLOCK_SIZE][j] += D2[i][j];
                        }
                    }
                    ptr->jobList[ptr->rear].statusDone = 6;
                    printf("Worker Process\nMatrix ID: %d\nWork Number: %d\n Status: %d\n\n", ptr->jobList[ptr->rear].matId, work + 1, ptr->jobList[ptr->rear].statusDone);
                }
                else if (currentStatus == 6)
                {
                    for (int i = BLOCK_SIZE; i < MATRIX_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            D3[i][j] = 0;
                            for (int k = 0; k < BLOCK_SIZE; k++)
                                D3[i][j] += ptr1[i][k+BLOCK_SIZE] * ptr2[k+BLOCK_SIZE][j+BLOCK_SIZE];
                        }
                    }
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            ptr->jobList[ptr->rear].matrix[i+BLOCK_SIZE][j] += D2[i][j];
                        }
                    }
                    ptr->jobList[ptr->rear].statusDone = 7;
                    printf("Worker Process\nMatrix ID: %d\nWork Number: %d\n Status: %d\n\n", ptr->jobList[ptr->rear].matId, work + 1, ptr->jobList[ptr->rear].statusDone);
                }
                else if (currentStatus == 7)
                {
                    for (int i = BLOCK_SIZE; i < MATRIX_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            D3[i][j] = 0;
                            for (int k = 0; k < BLOCK_SIZE; k++)
                                D3[i][j] += ptr1[i][k+BLOCK_SIZE] * ptr2[k+BLOCK_SIZE][j+BLOCK_SIZE];
                        }
                    }
                    for (int i = 0; i < BLOCK_SIZE; i++)
                    {
                        for (int j = 0; j < BLOCK_SIZE; j++)
                        {
                            ptr->jobList[ptr->rear].matrix[i+BLOCK_SIZE][j] += D2[i][j];
                        }
                    }
                    ptr->jobList[ptr->rear].statusDone = 8;
                    printf("Worker Process\nMatrix ID: %d\nWork Number: %d\n Status: %d\n\n", ptr->jobList[ptr->rear].matId, work + 1, ptr->jobList[ptr->rear].statusDone);
                }
                removeJob(ptr);
                removeJob(ptr);
            }
            // Detach memory segment
            shmdt(memSegment);
            exit(0);
        }
    }
    // Complete all tasks
    while (*jobs_created < numMat || ptr->currSize != 1);

    end = clock();
    double total_duration = (double)(end - start)/CLOCKS_PER_SEC;
    printf("\nTotal duration to run the program: %d\n", total_duration);

    shmdt(memSegment);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
