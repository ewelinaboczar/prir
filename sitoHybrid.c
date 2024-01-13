#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// mpicc -fopenmp sitoHybrid.c -o sitoHybrid -lm
// mpirun --allow-run-as-root -np 5 ./sitoHybrid <liczba_watkow> <liczba>

// Definicje stałych do obliczeń bloków
#define FIRST_PRIME 3
#define PRIME_STEP 2

// Makra dla obliczeń bloków
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define BLOCK_LOW(id, p, n) ((id) * (n) / (p) / PRIME_STEP)
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id) + 1, p, n) - BLOCK_LOW((id), p, n))
#define BLOCK_OWNER(index, p, n) (((p) * ((index) + 1) - 1) / (n))
#define BLOCK_VALUE_TO_INDEX(val, id, p, n) ((val - FIRST_PRIME) / PRIME_STEP - BLOCK_LOW(id, p, n - 1))


int main(int argc, char** argv) {
    int local_count;        // lokalna liczba liczb pierwszych
    double elapsed_time;    // czas wykonania równoległego programu
    int first_value;        // indeks pierwszej wartości w bloku
    int global_count;       // globalna liczba liczb pierwszych
    int high_value;         // najwyższa wartość przetwarzana przez proces
    int process_id;         // identyfikator procesu
    int low_value;          // najniższa wartość przetwarzana przez proces
    int n;                  // górny zakres liczb pierwszych
    int num_processes;      // liczba procesów
    int proc0_size;         // rozmiar podtablicy procesu 0
    int prime;              // aktualna liczba pierwsza
    int block_size;         // liczba elementów w oznaczonej tablicy
    int sqrt_n;             // pierwiastek kwadratowy z n
    int multiple_of_prime;  // wielokrotność liczby pierwszej
    int num_per_block;      // liczba elementów przetwarzanych w bloku
    int block_low_value;    // najniższa wartość w bloku
    int block_high_value;   // najwyższa wartość w bloku
    int first_index_in_block;  // indeks pierwszej wartości w bloku
    bool* marked;           // tablica oznaczająca liczby do skreślenia
    bool* primes;           // tablica oznaczająca liczby pierwsze

    int numThreads;

    // Sprawdzenie, czy podano odpowiednią liczbę argumentów
    if (argc != 3) {
        if (process_id == 0)
            printf("Użycie: %s <liczba_wątków> <liczba>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    // Pobranie wartości z argumentów
    n = atoi(argv[2]);
    numThreads = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    elapsed_time = -MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);



    // Obliczenia dotyczące bloków i rozmiaru
    low_value = FIRST_PRIME + BLOCK_LOW(process_id, num_processes, n - 1) * PRIME_STEP;
    high_value = FIRST_PRIME + BLOCK_HIGH(process_id, num_processes, n - 1) * PRIME_STEP;
    block_size = BLOCK_SIZE(process_id, num_processes, n - 1);

    // Sprawdzenie warunku dotyczącego procesów
    proc0_size = (n - 1) / num_processes;
    if ((2 + proc0_size) < (int)sqrt((double)n)) {
        if (process_id == 0)
            printf("Zbyt wiele procesów\n");
        MPI_Finalize();
        exit(1);
    }

    // Inicjalizacja tablicy primes do oznaczania liczb pierwszych
    sqrt_n = sqrt(n);
    primes = (bool*)calloc(sqrt_n + 1, sizeof(bool));

    #pragma omp parallel for num_threads(numThreads)
    for (multiple_of_prime = 2; multiple_of_prime <= sqrt_n; multiple_of_prime += 2) {
        primes[multiple_of_prime] = true;
    }

    #pragma omp parallel for num_threads(numThreads)
    for (prime = 3; prime <= sqrt_n; prime += 2) {
        if (primes[prime])
            continue;

        #pragma omp parallel for num_threads(numThreads) schedule(static)
        for (multiple_of_prime = prime << 1; multiple_of_prime <= sqrt_n; multiple_of_prime += prime) {
            primes[multiple_of_prime] = true;
        }
    }

    // Inicjalizacja tablicy marked do oznaczania liczb do skreślenia
    marked = (bool*)calloc(block_size, sizeof(bool));
    if (marked == NULL) {
        printf("Nie można zaalokować wystarczającej ilości pamięci\n");
        MPI_Finalize();
        exit(1);
    }

    // Parametry optymalizacyjne
    num_per_block = 1024 * 1024;
    block_low_value = low_value;
    block_high_value = MIN(high_value, low_value + num_per_block * PRIME_STEP);

    // Iteracja przez bloki
    #pragma omp parallel for num_threads(numThreads) schedule(dynamic)
    for (first_index_in_block = 0; first_index_in_block < block_size; first_index_in_block += num_per_block) {
        // Iteracja po liczbach pierwszych
        #pragma omp parallel for num_threads(numThreads) schedule(dynamic)
        for (prime = 3; prime <= sqrt_n; prime++) {
            if (primes[prime])
                continue;

            if (prime * prime > block_low_value) {
                first_value = prime * prime;
            } else {
                if (!(block_low_value % prime)) {
                    first_value = block_low_value;
                } else {
                    first_value = prime - (block_low_value % prime) + block_low_value;
                }
            }

            if ((first_value + prime) & 1)
                first_value += prime;

            int first_index = BLOCK_VALUE_TO_INDEX(first_value, process_id, num_processes, n - 1);
            int prime_doubled = prime << 1;
            int prime_step = prime_doubled / PRIME_STEP;

            // Oznaczanie liczb do skreślenia
            for (int i = first_value; i <= high_value; i += prime_doubled) {
                marked[first_index] = true;
                first_index += prime_step;
            }
        }

        block_low_value += num_per_block * PRIME_STEP;
        block_high_value = MIN(high_value, block_high_value + num_per_block * PRIME_STEP);
    }

    // Zliczanie liczb pierwszych na danym procesie
    local_count = 0;
    #pragma omp parallel for num_threads(numThreads) reduction(+:local_count)
    for (int i = 0; i < block_size; i++)
        if (!marked[i])
            local_count++;

    // Redukcja sumy liczb pierwszych na wszystkich procesach
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Zatrzymanie timera
    elapsed_time += MPI_Wtime();

    // Wyświetlenie wyników
    if (process_id == 0) {
        global_count += 1; // dodaj pierwszą liczbę pierwszą, 2
        printf("%d liczb pierwszych jest mniejszych lub równych %d\n", global_count, n);
        printf("Całkowity czas wykonania: %10.6fs\n", elapsed_time);
    }

    // Zwolnienie pamięci
    free(marked);
    free(primes);

    MPI_Finalize();

    return 0;
}