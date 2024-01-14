#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include <time.h>

// gcc -fopenmp sitoOpenMP.c -o sitoOMP -lm
// ./sitoOMP <liczba_wątków> <liczba>

int eratosthenesSieve(int n, int numThreads) {
    // Włączanie OpenMP z zadaną liczbą wątków
    omp_set_num_threads(numThreads);

    int found;

    if (n >= 2)
        found = 1;
      else
        found = 0;

    // Ograniczenie warunku w pętli dla optymalizacji OpenMP (zamiast i*i <= n piszemy i <k= nSqrt)
    const int nSqrt = (int)sqrt((double)n);

    // Alokacja pamięci
    int alloc = (n - 1) / 2;
    char *prime = (char*)malloc((alloc + 1) * sizeof(char));

    // Ustawianie wszystkich wartości jako liczby pierwsze
    #pragma omp parallel for
    for (int i = 0; i <= alloc; i++)
        prime[i] = 1;

    // Algorytm sita Eratostenesa - ustawianie wielokrotności liczb na niepierwsze
    #pragma omp parallel for schedule(dynamic)
    for (int i = 3; i <= nSqrt; i += 2)
        if (prime[i / 2])
            for (int j = i * i; j <= n; j += 2 * i)
                prime[j / 2] = 0;

    // Zliczanie liczb pierwszych
    #pragma omp parallel for reduction(+:found)
    for (int i = 1; i <= alloc; i++)
        found += prime[i];

    free(prime);

    return found;
}

int main(int argc, char** argv) {
    int n = 0;
    int numThreads = 0;
    int primeCount = 0;
    double cpu_time_used;
    clock_t start, end;

    // Sprawdzenie, czy podano odpowiednią liczbę argumentów
    if (argc != 3) {
        printf("Użycie: %s <liczba_wątków> <liczba>\n", argv[0]);
        return 1;
    }

    // Pobranie wartości z argumentów
    numThreads = atoi(argv[1]);
    n = atoi(argv[2]);

    // Pomiar czasu
    start = clock();
    primeCount = eratosthenesSieve(n, numThreads);
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("%d liczb pierwszych jest mniejszych lub rownych %d\n", primeCount, n);
    printf("Czas wykonania: %10.6f sekundy\n", cpu_time_used);

    return 0;
}
