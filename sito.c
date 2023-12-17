#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// gcc -fopenmp sito.c -o sito
// ./sito

void sitoEratostenesa(int n, bool prime[]) {
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
}

void sitoEratostenesaOpenMP(int n, bool prime[]) {
    #pragma omp parallel for num_threads(4)
    for (int i = 0; i <= n; i++) {
        prime[i] = true;
    }

    // Algorytm sita Eratostenesa
    #pragma omp parallel num_threads(4)
    for (int p = 2; p * p <= n; p++) {
        if (prime[p] == true) {
            bool local_prime_p = prime[p];
            #pragma omp for
            for (int i = p * p; i <= n; i += p) {
                prime[i] = false;
            }
        }
    }
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
    int values[] = {300000, 500000, 1000000, 1500000, 2000000, 50000000};


    printf("Sekwancyjnie\n--------------------------------------------\n");

    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        double cpu_time_used;
        clock_t start = 0, end = 0;

        bool *prime = (bool *)malloc((values[i] + 1) * sizeof(bool));
        if (prime == NULL) {
            fprintf(stderr, "Błąd alokacji pamięci.\n");
            return 1;
        }

        start = clock();
        sitoEratostenesa(values[i], prime);
        end = clock();

        //printSito(values[i], prime);

        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Czas wykonania dla n = %d: %f sekundy\n", values[i], cpu_time_used);

        free(prime);
    }


    printf("\n\nOpenMP\n--------------------------------------------\n");

    for (int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        double cpu_time_used;
        clock_t start = 0, end = 0;

        bool *prime = (bool *)malloc((values[i] + 1) * sizeof(bool));
        if (prime == NULL) {
            fprintf(stderr, "Błąd alokacji pamięci.\n");
            return 1;
        }

        start = clock();
        sitoEratostenesaOpenMP(values[i], prime);
        end = clock();

        //printSito(values[i], prime);

        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Czas wykonania dla n = %d: %f sekundy\n", values[i], cpu_time_used);

        free(prime);
       }

    return 0;
}