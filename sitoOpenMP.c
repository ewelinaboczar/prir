#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <math.h>

#define sieve_num size_t

void sitoEratostenesaOpenMP(int n, bool prime[], int numThreads, float max_n) {
    omp_set_num_threads(numThreads);

    // Algorytm sita Eratostenesa
    #pragma omp parallel for firstprivate(max_n) firstprivate(prime) schedule(static)
    for (size_t p = 2; p <= (size_t)max_n; p++) {
        if (!prime[p]) {
            #pragma omp parallel for firstprivate(p) schedule(static)
            for (int i = p * p; i <= n; i += p) {
                prime[i] = true;
            }
        }
    }
}

void printSito(int n, bool prime[]) {
    printf("Liczby pierwsze do %d:\n", n);
    for (size_t p = 2; p <= n; p++) {
        if (!prime[p]) {
            printf("%ld ", p);
        }
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <liczba_wątków> <n>\n", argv[0]);
        return 1;
    }

    int numThreads = atoi(argv[1]);
    int n = atoi(argv[2]);
    size_t max_n = sqrtf((size_t)n);
    size_t global_count = 0;

    bool *prime = (bool *)malloc((n + 1) * sizeof(bool));
    if (prime == NULL) {
        fprintf(stderr, "Błąd alokacji pamięci.\n");
        return 1;
    }

    double start = omp_get_wtime();
    sitoEratostenesaOpenMP(n, prime, numThreads, max_n);
    double end = omp_get_wtime();

    //printSito(n, prime);

    for (int i = 2; i <= n; i++) {
            if (!prime[i]) {
                global_count++;
            }
        }

    printf("%zu liczb pierwszych jest mniejszych lub równych %d\n", global_count, n);
    printf("Całkowity czas wykonania: %10.6fs\n", end - start);

    free(prime);
    return 0;
}
