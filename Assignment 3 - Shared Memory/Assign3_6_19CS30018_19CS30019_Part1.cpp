#include <iostream>
using namespace std;
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

typedef struct _process_Data
{
	double **A;
	double **B;
	double **C;
	int veclen, i, j;
} ProcessData;

// compute A[i,:]*B[:,j] and store in C[i][j];
void mult(ProcessData *data)
{
	data->C[data->i][data->j] = 0;
	for (int n = 0; n < data->veclen; n++)
	{
		data->C[data->i][data->j] += data->A[data->i][n] * data->B[n][data->j];
	}

	return;
}

int main()
{
	int shmid1, shmid2, pid, status;
	double *shmdata;
	double *ptr;
	double **mtx1, **mtx2, **mtx3;
	int i, j, k, r1, r2, c1, c2;

	// reading matrix 1
	printf("Enter the row r1 and column c1 of matrix 1: ");
	scanf("%d%d", &r1, &c1);
	mtx1 = (double **)malloc(sizeof(double) * r1 * c1);

	// reading matrix 2
	printf("\nEnter the row r2 and column c2 of matrix 2: ");
	scanf("%d%d", &r2, &c2);

	// row of matrix 1 != column of matrix 2
	if (r2 != c1)
	{
		printf("\nError: column of 1st matrix and row of 2nd matrix should be same.\n");
		return 0;
	}
	mtx2 = (double **)malloc(sizeof(double) * r2 * c2);

	// ftok to generate unique key
	key_t key = ftok("shmfile", 65);

	// shmget returns an identifier in shmid
	int memsize = r1 * c2 + r1 * c1 + r2 * c2;
	int shmid = shmget(key, memsize, 0666 | IPC_CREAT);
	if (shmid == -1)
	{
		printf("Error allocating shared memory\n ");
		exit(1);
	}

	// shmat to attach to shared memory
	shmdata = (double *)shmat(shmid, (void *)0, 0);
	ptr = (double *)shmdata; // pointer to the shared memory

	printf("Enter the matrix 1: \n");

	for (i = 0; i < r1; i++)
	{
		mtx1[i] = ptr;
		for (j = 0; j < c1; j++)
		{
			scanf("%lf", &mtx1[i][j]);
		}
		ptr += c1;
	}

	printf("Enter the matrix 2: \n");
	for (i = 0; i < r2; ++i)
	{
		mtx2[i] = ptr;
		for (j = 0; j < c2; j++)
			scanf("%lf", &mtx2[i][j]);
		ptr += c2;
	}

	mtx3 = (double **)malloc(sizeof(double *) * r1);
	for (int i = 0; i < c1; i++)
	{
		mtx3[i] = ptr;
		ptr += c2;
	}

	for (int i = 0; i < r1; i++)
	{
		for (int j = 0; j < c2; j++)
		{
			int pid = fork();
			if (pid == -1)
			{
				printf("Error: In forking");
				exit(0);
			}
			else if (pid == 0)
			{
				ProcessData *arg = (ProcessData *)malloc(sizeof(ProcessData));
				arg->A = mtx1;
				arg->B = mtx2;
				arg->C = mtx3;
				arg->j = j;
				arg->veclen = r2;
				arg->i = i;
				mult(arg);
				shmdt(shmdata); // detach from child
				exit(1);
			}
		}
	}

	// wait for all child processes
	for (int i = 0; i < r1 * c2; i++)
		wait(NULL);

	printf("\nmatrix C is \n");
	for (i = 0; i < r1; ++i)
	{
		printf("\n");
		for (j = 0; j < c2; j++)
			printf("%lf ", mtx3[i][j]);
	}

	printf("\n");

	shmdt(shmdata); // detach from parent
	shmctl(shmid, IPC_RMID, NULL);
	free(mtx1);
	free(mtx2);
	free(mtx3);

	return 0;
}