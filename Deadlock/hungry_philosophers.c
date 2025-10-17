// EXEMPLO DO ZAMITH

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 10       // número de filósofos
#define STEPS 100
//#define THINKING 0
//#define HUNGRY   1
//#define EATING   2

omp_lock_t garfos[N];   // cada garfo é um lock

void pensar(int i) {
    printf("Filósofo %d está pensando...\n", i);
    
}

void comer(int i) {
    printf("Filósofo %d está comendo!\n", i);
}

int main() {
    int i;

    // inicializa locks (garfos)
    for (i = 0; i < N; i++) {
        omp_init_lock(&garfos[i]);
    }

    // executa filósofos em paralelo
    #pragma omp parallel num_threads(N) private(i)
    {
        i = omp_get_thread_num();

        for (int k = 0; k < STEPS; k++) { // cada filósofo come 3 vezes
            pensar(i);

            // pega garfo à esquerda
            omp_set_lock(&garfos[i]);
            // pega garfo à direita (com cuidado para evitar deadlock)
            omp_set_lock(&garfos[(i+1)%N]);

            comer(i);

            // solta os garfos
            omp_unset_lock(&garfos[i]);
            omp_unset_lock(&garfos[(i+1)%N]);
        }
    }

    // destrói locks
    for (i = 0; i < N; i++) {
        omp_destroy_lock(&garfos[i]);
    }

    return 0;
}