# Przetwarzanie Współbieżne
# Sito Eratostenesa

## 1. Uruchomienie i kompilacja sitoSeq.c:

 _gcc sitoSeq.c -o sitoSeq_
 _./sitoSeq_

### 2. Uruchomienie i kompilacja sitoHybrid.c:

 _mpicc sitoMPI.c -o sitoMPI -lm_
 _mpirun --allow-run-as-root -np 5 ./sitoMPI <liczba>_

### 3. Uruchomienie i kompilacja sitoOpenMP.c:

 _gcc -fopenmp sitoOpenMP.c -o sitoOpenMP -lm_
 _./sitoOpenMP <liczba_watkow> <liczba>_

## 4. Uruchomienie i kompilacja sitoHybrid.c:

 _mpicc -fopenmp sitoHybrid.c -o sitoHybrid -lm_
 _mpirun --allow-run-as-root -np 5 ./sitoHybrid <liczba_watkow> <liczba>_
