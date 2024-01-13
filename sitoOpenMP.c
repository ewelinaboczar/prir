#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <omp.h>
#include <time.h>

// gcc -fopenmp sitoOpenMP.c -o sito
// ./sito

// Funkcja realizująca sito Eratostenesa dla liczb nieparzystych
int eratosthenesOdd(int lastNumber, bool useOpenMP) {
    // Włączanie/wyłączanie OpenMP
    omp_set_num_threads(useOpenMP ? omp_get_num_procs() : 1);

    // Ograniczenie warunku w pętli dla optymalizacji OpenMP
    const int lastNumberSqrt = (int)sqrt((double)lastNumber);
    int memorySize = (lastNumber - 1) / 2;

    // Inicjalizacja tablicy isPrime, reprezentującej liczby pierwsze
    char* isPrime = (char*)malloc((memorySize + 1) * sizeof(char));

    // Ustawienie wszystkich wartości na 1 (liczba pierwsza)
    #pragma omp parallel for
    for (int i = 0; i <= memorySize; i++)
        isPrime[i] = 1;

    // Wykreślanie liczb nieparzystych, które nie są pierwsze
    #pragma omp parallel for schedule(dynamic)
    for (int i = 3; i <= lastNumberSqrt; i += 2)
        if (isPrime[i / 2])
            for (int j = i * i; j <= lastNumber; j += 2 * i)
                isPrime[j / 2] = 0;

    // Obliczanie liczby liczb pierwszych
    int found = lastNumber >= 2 ? 1 : 0;

    #pragma omp parallel for reduction(+:found)
    for (int i = 1; i <= memorySize; i++)
        found += isPrime[i];

    // Zwolnienie zaalokowanej pamięci
    free(isPrime);
    return found;
}

int main() {
    int lastNumber = 100;
    bool useOpenMP = true;

    clock_t start, end;
    double cpu_time_used;

    // Pomiar czasu rozpoczyna się
    start = clock();
    // Wywołanie funkcji
    int primeCount = eratosthenesOdd(lastNumber, useOpenMP);
    // Pomiar czasu kończy się
    end = clock();

    // Obliczenie czasu wykonywania programu w sekundach
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    // Wyświetlenie wyników
    printf("Liczba liczb pierwszych mniejszych lub równych %d: %d\n", lastNumber, primeCount);
    printf("Czas wykonania: %f sekundy\n", cpu_time_used);

    return 0;
}
