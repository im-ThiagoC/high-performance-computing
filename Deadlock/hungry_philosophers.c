#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 10
#define STEPS 100

pthread_mutex_t garfos[N];

void pensar(int i) {
  struct timespec ts = {1, 0};
  printf("Filosofo %d esta pensando...\n", i);
  nanosleep(&ts, NULL);
}

void comer(int i) {
  struct timespec ts = {1, 0};
  printf("Filosofo %d esta comendo...\n", i);
  nanosleep(&ts, NULL);
}

void* filosofo(void* arg) {
  int i = *(int*)arg;
  for (int k = 0; k < STEPS; k++) {
    pensar(i);

    // pega garfo à esquerda
    pthread_mutex_lock(&garfos[i]);
    // pega garfo à direita
    pthread_mutex_lock(&garfos[(i+1)%N]);

    comer(i);

    // solta os garfos
    pthread_mutex_unlock(&garfos[i]);
    pthread_mutex_unlock(&garfos[(i+1)%N]);
  }
  return NULL;
}

int main() {
  pthread_t threads[N];
  int ids[N];

  for (int i = 0; i < N; i++) {
    pthread_mutex_init(&garfos[i], NULL);
  }

  for (int i = 0; i < N; i++) {
    ids[i] = i;
    pthread_create(&threads[i], NULL, filosofo, &ids[i]);
  }

  for (int i = 0; i < N; i++) {
    pthread_join(threads[i], NULL);
  }

  for (int i = 0; i < N; i++) {
    pthread_mutex_destroy(&garfos[i]);
  }

  printf("Todos os filosofos terminaram suas refeicoes.\n");

  return 0;
}
