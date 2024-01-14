#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// gcc sitoSeq.c -o sitoSeq
// ./sitoSeq

// Wypisywanie liczb pierwszych
void printSito(int n, bool prime[]) {

    printf("Liczby pierwsze do %d:\n", n);

    for (int p = 2; p <= n; p++) {
        if (prime[p]) {
            printf("%d ", p);
        }
    }

    printf("\n");
}

int eratosthenesSieve(int n) {

    int found = 0;

    // Alokacja pamięci
    bool *prime = (bool *)malloc((n + 1) * sizeof(bool));
    if (prime == NULL) {
        fprintf(stderr, "Błąd alokacji pamięci.\n");
        return 1;
    }

    // Ustawianie wszystkich wartości jako liczby pierwsze
    for (int i = 0; i <= n; i++) {
        prime[i] = true;
    }

    // Algorytm sita Eratostenesa - ustawianie wielokrotności liczb na niepierwsze
    for (int p = 2; p * p <= n; p++) {
        if (prime[p] == true) {
            for (int i = p * p; i <= n; i += p) {
                prime[i] = false;
            }
        }
    }

    // Zliczanie liczb pierwszych
    for (int i = 2; i <= n; i++) {
        if (prime[i])
            found++;
    }

    //printSito(n, prime);
    free(prime);

    return found;
}

int main(int argc, char *argv[]) {

    // Wykonanie algorytmu dla roznych zakresow
    int values[] = {1000, 50000, 300000, 500000, 1000000, 1500000, 2000000, 10000000};

    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        int found = 0;
        double cpu_time_used;
        clock_t start = 0, end = 0;

        // Pomiar czasu
        start = clock();
        found = eratosthenesSieve(values[i]);
        end = clock();

        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        printf("\n%d liczb pierwszych jest mniejszych lub rownych %d\n", found, values[i]);
        printf("Czas wykonania dla n = %d: %10.6f sekundy\n", values[i], cpu_time_used);
    }

    return 0;
}