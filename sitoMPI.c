#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

// mpicc sitoMPI.c -o sitoMPI -lm
// mpirun --allow-run-as-root -np 3 ./sitoMPI




void sitoEratostenesaMPI(int n) {
    int *marked, i, j;

    marked = (int *)malloc((n + 1) * sizeof(int));

    for (i = 2; i <= n; i++)
        marked[i] = 1;

    for (i = 2; i <= sqrt(n); i++) {
        if (marked[i]) {
            for (j = i * i; j <= n; j += i)
                marked[j] = 0;
        }
    }

    free(marked);  // Zwolnienie pamięci przed zakończeniem funkcji
}

int main(int argc, char **argv) {
    int values[] = {300000, 500000, 1000000, 1500000, 2000000, 50000000};


    //printf("MPI\n--------------------------------------------\n");

    int rank, size;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        int n = values[i];

        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();

        sitoEratostenesaMPI(n);

        end_time = MPI_Wtime();
        MPI_Barrier(MPI_COMM_WORLD);

        if (rank == 0) {
            printf("Czas wykonania dla n = %d: %f sekundy\n", n, end_time - start_time);
        }
    }

    MPI_Finalize();

    return 0;
}
