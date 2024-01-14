#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// mpicc sitoMPI.c -o sitoMPI -lm
// mpirun --allow-run-as-root -np 5 ./sitoMPI <liczba>

#define FIRST_PRIME 3
#define PRIME_STEP 2

// Makra dla obliczeń bloków
#define BLOCK_LOWEST(id, p, n) ((id) * (n) / (p) / PRIME_STEP)
#define BLOCK_HIGHEST(id, p, n) (BLOCK_LOWEST((id) + 1, p, n) - 1)
#define BLOCK_FIND_INDEX_OF_VAL(val, id, p, n) ((val - FIRST_PRIME) / PRIME_STEP - BLOCK_LOWEST(id, p, n - 1))
#define BLOCK_SIZE(id, p, n) (BLOCK_LOWEST((id) + 1, p, n) - BLOCK_LOWEST((id), p, n))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


int main(int argc, char** argv) {
    int proc_count_n;            // lokalna liczba liczb pierwszych
    int block_first_value;       // indeks pierwszej wartości w bloku
    int global_count_n;          // globalna liczba liczb pierwszych
    int proc_highest_value;      // najwyższa wartość przetwarzana przez proces
    int process_id;              // identyfikator procesu
    int proc_lowest_value;       // najniższa wartość przetwarzana przez proces
    int n;                       // górny zakres liczb pierwszych
    int num_processes;           // liczba procesów
    int proc0_size;              // rozmiar podtablicy procesu 0
    int current_prime;           // aktualna liczba pierwsza
    int block_size;              // liczba elementów w oznaczonej tablicy
    int n_sqrt;                  // pierwiastek kwadratowy z n
    int prime_multiplication;    // wielokrotność liczby pierwszej
    int num_per_block;           // liczba elementów przetwarzanych w bloku
    int block_lowest_value;      // najniższa wartość w bloku
    int block_highest_value;     // najwyższa wartość w bloku
    int block_first_index;       // indeks pierwszej wartości w bloku
    bool* marked;                // tablica oznaczająca liczby do skreślenia
    bool* primes;                // tablica oznaczająca liczby pierwsze
    double time_used;            // czas wykonania równoległego programu

    MPI_Init(&argc, &argv);
    time_used = -MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    if (argc != 2) {
        if (process_id == 0)
            printf("Użycie: %s <liczba>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    // Pobranie wartości z argumentów
    n = atoi(argv[1]);

    // Obliczenia dotyczące bloków i rozmiaru
    proc_lowest_value = FIRST_PRIME + BLOCK_LOWEST(process_id, num_processes, n - 1) * PRIME_STEP;
    proc_highest_value = FIRST_PRIME + BLOCK_HIGHEST(process_id, num_processes, n - 1) * PRIME_STEP;
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
    n_sqrt = sqrt(n);
    primes = (bool*)calloc(n_sqrt + 1, sizeof(bool));
    for (prime_multiplication = 2; prime_multiplication <= n_sqrt; prime_multiplication += 2) {
        primes[prime_multiplication] = true;
    }

    for (current_prime = 3; current_prime <= n_sqrt; current_prime += 2) {
        if (primes[current_prime])
            continue;

        for (prime_multiplication = current_prime << 1; prime_multiplication <= n_sqrt; prime_multiplication += current_prime) {
            primes[prime_multiplication] = true;
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
    block_lowest_value = proc_lowest_value;
    block_highest_value = MIN(proc_highest_value, proc_lowest_value + num_per_block * PRIME_STEP);

    // Iteracja przez bloki
    for (block_first_index = 0; block_first_index < block_size; block_first_index += num_per_block) {
        // Iteracja po liczbach pierwszych
        for (current_prime = 3; current_prime <= n_sqrt; current_prime++) {
            if (primes[current_prime])
                continue;

            if (current_prime * current_prime > block_lowest_value) {
                block_first_value = current_prime * current_prime;
            } else {
                if (!(block_lowest_value % current_prime)) {
                    block_first_value = block_lowest_value;
                } else {
                    block_first_value = current_prime - (block_lowest_value % current_prime) + block_lowest_value;
                }
            }

            if ((block_first_value + current_prime) & 1)
                block_first_value += current_prime;

            int first_index = BLOCK_FIND_INDEX_OF_VAL(block_first_value, process_id, num_processes, n - 1);
            int prime_doubled = current_prime << 1;
            int prime_step = prime_doubled / PRIME_STEP;

            // Oznaczanie liczb do skreślenia
            for (int i = block_first_value; i <= proc_highest_value; i += prime_doubled) {
                marked[first_index] = true;
                first_index += prime_step;
            }
        }

        block_lowest_value += num_per_block * PRIME_STEP;
        block_highest_value = MIN(proc_highest_value, block_highest_value + num_per_block * PRIME_STEP);
    }

    // Zliczanie liczb pierwszych na danym procesie
    proc_count_n = 0;
    for (int i = 0; i < block_size; i++)
        if (!marked[i])
            proc_count_n++;

    // Redukcja sumy liczb pierwszych na wszystkich procesach i zatrzymanie pomiaru czasu
    MPI_Reduce(&proc_count_n, &global_count_n, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    time_used += MPI_Wtime();

    if (process_id == 0) {
        global_count_n += 1; // dodaj pierwszą liczbę pierwszą, 2
        printf("%d liczb pierwszych jest mniejszych lub równych %d\n", global_count_n, n);
        printf("Czas wykonania: %10.6fs\n", time_used);
    }

    free(marked);
    free(primes);
    MPI_Finalize();

    return 0;
}