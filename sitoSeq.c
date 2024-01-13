#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// gcc sitoSeq.c -o sito
// ./sito

int sitoEratostenesa(int n, bool prime[]) {
    int found = 0;

    for (int i = 0; i <= n; i++) {
        prime[i] = true;
    }

    // Algorytm sita Eratostenesa
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

    return found;
}


void printSito(int n, bool prime[]) {
    printf("Liczby pierwsze do %d:\n", n);
    for (int p = 2; p <= n; p++) {
        if (prime[p]) {
            printf("%d ", p);
        }
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int values[] = {1000, 50000, 300000, 500000, 1000000, 1500000, 2000000, 10000000};

    printf("Sekwancyjnie\n--------------------------------------------\n");

    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        int found = 0;
        double cpu_time_used;
        clock_t start = 0, end = 0;

        bool *prime = (bool *)malloc((values[i] + 1) * sizeof(bool));
        if (prime == NULL) {
            fprintf(stderr, "Błąd alokacji pamięci.\n");
            return 1;
        }

        start = clock();
        found = sitoEratostenesa(values[i], prime);
        end = clock();

        //printSito(values[i], prime);

        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Liczba liczb pierwszych mniejszych lub równych %d: %d\n", values[i], found);
        printf("Czas wykonania dla n = %d: %f sekundy\n", values[i], cpu_time_used);

        free(prime);
    }

    return 0;
}