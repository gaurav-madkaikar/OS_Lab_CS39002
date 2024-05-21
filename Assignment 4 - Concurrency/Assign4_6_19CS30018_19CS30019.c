// Group 6
// Gaurav Madkaikar
// Girish Kumar
// OS Lab Assignment 4

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>

// Effective size of the tree will depend on the number of producer threads(P)
// Max. possible size of tree = 500 + (P * 100)
#define MAX_TREE_SIZE 1000
// 10^8
#define UPP_LIMIT 100000000
#define MAXSIZE 500100
#define MAX_DEPENDENT 20

typedef struct Node
{
    int job_Id;
    int job_Num;
    time_t complete;
    int dependentNodes[MAX_DEPENDENT];

    // Lock
    pthread_mutex_t lock;
    pthread_mutexattr_t att;

    // Additional Information
    int parentId;
    int dependencySize;
    int status;
    int newDepSize;
} node;

typedef struct sharedSeg
{
    pthread_mutex_t lock;
    pthread_mutexattr_t att;
    node tree[MAX_TREE_SIZE];
} dataSeg;

typedef struct args
{
    int runTime;
    int job_Num;
    node *tree;
    dataSeg *shMem;
    int *visit;
    int threadId;
} prodArgs;

int startTime, endTime, shmid;
int jobsCreated = 0, totJobsCreated = 0;
int numJobs = 0;
int visited[MAX_TREE_SIZE];

// Initialise new jobs
void initJob(node *newJob)
{
    newJob->job_Id = (rand() % UPP_LIMIT) + 1;
    newJob->status = 0;
    newJob->job_Num = jobsCreated;
    newJob->complete = (rand() % 250) + 1;
    newJob->dependencySize = 0;
    newJob->parentId = -1;
    memset(newJob->dependentNodes, -1, sizeof(newJob->dependentNodes));

    return;
}
// Create the initial tree
void init_Tree(node *treePtr)
{
    while (jobsCreated < numJobs)
    {
        // Create new job
        node newJob;
        initJob(&newJob);

        // Add job to the tree
        int currJobNum = jobsCreated;
        int randomNode;
        if (jobsCreated)
        {
            randomNode = (rand() % (jobsCreated));
        }
        // First node (create root)
        if (jobsCreated == 0)
        {
            treePtr[0].job_Id = newJob.job_Id;
            treePtr[0].complete = newJob.complete;
            treePtr[0].parentId = -5;
            jobsCreated++;
            totJobsCreated++;
        }
        // Addition of a child to an existing parent
        else
        {
            // Add to the parent list
            treePtr[randomNode].dependentNodes[treePtr[randomNode].dependencySize] = currJobNum;
            treePtr[randomNode].dependencySize++;
            treePtr[randomNode].newDepSize++;
            // Add to the tree
            newJob.parentId = randomNode;
            treePtr[currJobNum] = newJob;
            jobsCreated++;
            totJobsCreated++;
        }
    }
    return;
}

void *producer(void *args)
{
    prodArgs *temp = (prodArgs *)args;
    int runTime = temp->runTime;
    int proc_Id = temp->job_Num;
    node *treePtr = temp->tree;
    int threadId = temp->threadId;
    int maxTreeSize = MAX_TREE_SIZE;
    dataSeg *shMem = temp->shMem;

    endTime = clock();

    // Run only for the specified time
    while ((((long double)(endTime - startTime) <= (long double)runTime * CLOCKS_PER_SEC)) && (totJobsCreated < MAX_TREE_SIZE))
    {
        int randomNode = (rand() % (jobsCreated));
        if (treePtr[randomNode].status == 1)
            continue;

        // Create new job
        node newJob;
        initJob(&newJob);

        // Add job to the tree
        int currJobNum = jobsCreated;
        pthread_mutex_lock(&(shMem->lock));
        // First node (create root)
        if (jobsCreated == 0)
        {
            // pthread_mutex_lock(&treePtr[0].lock);
            treePtr[0].job_Id = newJob.job_Id;
            treePtr[0].complete = newJob.complete;
            treePtr[0].parentId = -5;
            jobsCreated++;
            totJobsCreated++;
        }
        // Unconnected node
        else if (treePtr[randomNode].parentId == -1)
        {
            // Add to the root
            treePtr[0].dependentNodes[treePtr[0].dependencySize] = currJobNum;
            treePtr[0].dependencySize++;
            treePtr[0].newDepSize++;
            // Initialise the new process
            treePtr[currJobNum] = newJob;
            treePtr[currJobNum].parentId = 0;
            jobsCreated++;
            totJobsCreated++;
        }
        // Addition of a child to an existing parent
        else
        {
            // Add to the parent list
            treePtr[randomNode].dependentNodes[treePtr[randomNode].dependencySize] = currJobNum;
            treePtr[randomNode].dependencySize++;
            treePtr[randomNode].newDepSize++;
            // Add to the tree
            newJob.parentId = randomNode;
            treePtr[currJobNum] = newJob;
            jobsCreated++;
            totJobsCreated++;
        }
        printf("[+] Producer Thread %d: Job ID = %d, Job No = %d, Completion time = %ld msec\n", threadId, newJob.job_Id, totJobsCreated, newJob.complete);
        pthread_mutex_unlock(&(shMem->lock));

        // Sleep for 200-500 msec
        usleep(((rand() % 301) + 200) * 1000);
    }

    pthread_exit(0);
    return NULL;
}

void BFS(int level, node *treePtr)
{
    if ((jobsCreated <= 0) || (visited[level]))
        return;
    int numChildren = treePtr[level].newDepSize;
    if (numChildren == 0)
    {
        visited[level] = 1;
        // Execute children
        usleep(treePtr[level].complete * 1000);

        // Mark status and restore the node
        treePtr[level].status = 1;
        treePtr[treePtr[level].parentId].newDepSize--;
        jobsCreated--;

        printf("[-] Consumer Thread: Job ID = %d, Execution time = %ld msec\n", treePtr[level].job_Id, treePtr[level].complete);
        if (jobsCreated <= 0)
            return;
        return;
    }

    for (int i = 0; i < numChildren; i++)
    {
        int nextTrav = treePtr[level].dependentNodes[i];
        BFS(nextTrav, treePtr);
    }
    visited[level] = 1;
    // Execute parent
    usleep(treePtr[level].complete * 1000);

    // Mark status and restore the node
    treePtr[level].status = 1;
    treePtr[treePtr[level].parentId].newDepSize--;
    treePtr[level].newDepSize = 0;
    jobsCreated--;

    printf("[-] Consumer Thread: Job ID = %d, Job Num = %d, Execution time = %ld msec\n", treePtr[level].job_Id, level, treePtr[level].complete);
    if (jobsCreated <= 0)
        return;

    return;
}

void *consumer(void *args)
{
    prodArgs *temp = (prodArgs *)args;
    int runTime = temp->runTime;
    int proc_Id = temp->job_Num;
    node *treePtr = temp->tree;
    int threadId = temp->threadId;
    int maxTreeSize = MAX_TREE_SIZE;
    dataSeg *shMem = temp->shMem;

    // Traverse the tree in a recursive DFS manner and execute the processes in bottom-up manner
    while (jobsCreated > 0)
    {
        pthread_mutex_lock(&(shMem->lock));
        // Search for a job dependency
        BFS(0, treePtr);
        pthread_mutex_unlock(&(shMem->lock));
    }

    pthread_exit(0);
    return NULL;
}

int main(int argc, char *argv[])
{
    // Master Process A
    srand(time(0));
    int start = clock();

    int NP, NY;
    printf("Producer-Consumer Problem using threads:\n");
    printf("Enter the number of producer threads: ");
    scanf("%d", &NP);
    printf("Enter the number of consumer threads: ");
    scanf("%d", &NY);
    printf("\n");

    // Random number of jobs
    numJobs = (rand() % 201) + 300;
    int maxTreeSize = MAX_TREE_SIZE;

    // Allocate shared memory and attach the dependency tree to the shared memory segment
    char *memSeg;
    dataSeg *shMem;
    node *treePtr;
    // ftok to generate unique key
    key_t key = ftok("shmfile", 65);
    size_t size = sizeof(dataSeg);
    shmid = shmget(key, size, IPC_CREAT | 0666);
    memSeg = shmat(shmid, NULL, 0);
    shMem = (dataSeg *)((void *)memSeg);
    treePtr = &(shMem->tree[0]);

    pthread_mutexattr_init(&shMem->att);
    pthread_mutexattr_setrobust(&shMem->att, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_setpshared(&shMem->att, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shMem->lock, &shMem->att);

    for (int i = 0; i < MAX_TREE_SIZE; i++)
    {
        pthread_mutexattr_init(&treePtr[i].att);
        pthread_mutexattr_setrobust(&treePtr[i].att, PTHREAD_MUTEX_ROBUST);
        pthread_mutexattr_setpshared(&treePtr[i].att, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&treePtr[i].lock, &treePtr[i].att);
        treePtr[i].parentId = -1;
        treePtr[i].dependencySize = 0;
        treePtr[i].newDepSize = 0;
        memset(treePtr[i].dependentNodes, -1, sizeof(treePtr[i].dependentNodes));
        treePtr[i].status = 0; // Job not yet created
    }

    // Initialise the tree
    init_Tree(treePtr);

    pthread_t threadProducer[NP], threadConsumer[NY];

    /* Create independent threads each of which will execute function */
    for (int i = 0; i < NP; i++)
    {
        srand(time(0));
        prodArgs temp;
        int runTime = (rand() % 11) + 10;
        startTime = clock();
        temp.runTime = runTime;
        temp.job_Num = i;
        temp.tree = treePtr;
        temp.shMem = shMem;
        temp.threadId = i + 1;
        // Spawn independent producer threads
        pthread_create(&threadProducer[i], NULL, producer, (void *)&temp);

        usleep(1);
    }
    
    jobsCreated = totJobsCreated;
    int newpid = fork();
    if (newpid == -1)
    {
        perror("Fork()");
        exit(EXIT_FAILURE);
    }
    else if (newpid == 0)
    {
        for (int i = 0; i < NY; i++)
        {
            srand(time(0));
            prodArgs temp;
            int runTime = (rand() % 11) + 10;
            startTime = clock();
            temp.runTime = runTime;
            temp.job_Num = i;
            temp.tree = treePtr;
            temp.shMem = shMem;
            // Spawn independent consumer threads
            pthread_create(&threadConsumer[i], NULL, consumer, (void *)&temp);
        }
        for (int i = 0; i < NY; i++)
        {
            pthread_join(threadConsumer[i], NULL); // wait for all consumer threads to exit
        }
        exit(EXIT_SUCCESS);
    }
    for (int i = 0; i < NP; i++)
    {
        pthread_join(threadProducer[i], NULL); // wait for all producer threads to exit
    }

    // Print tree
    printf("\nFinal Tree as an Adjacency List:\n\n");
    for (int i = 0; i < maxTreeSize; i++)
    {
        if (treePtr[i].dependencySize > 0)
        {
            printf("Index %d: {", i);
            for (int j = 0; j < treePtr[i].dependencySize; j++)
            {
                if (treePtr[i].dependentNodes[j] > 0)
                    printf("%d", treePtr[i].dependentNodes[j]);
                if (j != treePtr[i].dependencySize - 1)
                    printf(" ");
            }
            printf("}\n");
        }
    }

    int end = clock();
    printf("\nTime of execution: %.3lf sec\n", (double)(end - start) / CLOCKS_PER_SEC);
    shmdt(memSeg);

    /* this structure is used by the shmctl() system call. */
    struct shmid_ds shm_desc;

    /* destroy the shared memory segment. */
    if (shmctl(shmid, IPC_RMID, &shm_desc) == -1)
    {
        perror("Destroy shared memory segment!");
        exit(EXIT_FAILURE);
    }
    return 0;
}