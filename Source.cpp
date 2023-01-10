#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define N 1000
#define TAG_MW 1
#define TAG_WM 6

void arrInit();

int size, rank;
int i, j, k;
double arr1[N][N], arr2[N][N], res[N][N];
double start, finish;
int bottom, top;
int part;
MPI_Status status;
MPI_Request req;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == 0) {
		arrInit();
		start = MPI_Wtime();
		for (i = 1; i < size; i++) {
			part = (N / (size - 1));
			bottom = (i - 1) * part;
			if (((i + 1) == size) && ((N % (size - 1)) != 0)) {
				top = N;
			}
			else {
				top = bottom + part;
			}
			MPI_Isend(&bottom, 1, MPI_INT, i, TAG_MW, MPI_COMM_WORLD, &req);
			MPI_Isend(&top, 1, MPI_INT, i, TAG_MW + 1, MPI_COMM_WORLD, &req);
			MPI_Isend(&arr1[bottom][0], (top - bottom) * N, MPI_DOUBLE, i, TAG_MW + 2, MPI_COMM_WORLD, &req);
		}
	}
	MPI_Bcast(&arr2, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	if (rank > 0) {
		MPI_Recv(&bottom, 1, MPI_INT, 0, TAG_MW, MPI_COMM_WORLD, &status);
		MPI_Recv(&top, 1, MPI_INT, 0, TAG_MW + 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&arr1[bottom][0], (top - bottom) * N, MPI_DOUBLE, 0, TAG_MW + 2, MPI_COMM_WORLD, &status);
		for (i = bottom; i < top; i++) {
			for (j = 0; j < N; j++) {
				for (k = 0; k < N; k++) {
					res[i][j] += (arr1[i][k] * arr2[k][j]);
				}
			}
		}
		MPI_Isend(&bottom, 1, MPI_INT, 0, TAG_WM, MPI_COMM_WORLD, &req);
		MPI_Isend(&top, 1, MPI_INT, 0, TAG_WM + 1, MPI_COMM_WORLD, &req);
		MPI_Isend(&res[bottom][0], (top - bottom) * N, MPI_DOUBLE, 0, TAG_WM + 2, MPI_COMM_WORLD, &req);
	}

	if (rank == 0) {
		for (i = 1; i < size; i++) {
			MPI_Recv(&bottom, 1, MPI_INT, i, TAG_WM, MPI_COMM_WORLD, &status);
			MPI_Recv(&top, 1, MPI_INT, i, TAG_WM + 1, MPI_COMM_WORLD, &status);
			MPI_Recv(&res[bottom][0], (top - bottom) * N, MPI_DOUBLE, i, TAG_WM + 2, MPI_COMM_WORLD, &status);
		}
		finish = MPI_Wtime();
		printf("\nRequired time: %f\n\n", finish - start);
	}
	MPI_Finalize();
	return 0;
}

void arrInit()
{
	srand(time(NULL));
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			arr1[i][j] = rand() / 10.0;
			arr2[i][j] = rand() / 10.0;
		}
	}
}