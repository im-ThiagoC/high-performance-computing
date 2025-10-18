// 1 produtor -> N consumidores
// Executar: ./producer_consumers [N_CONSUMIDORES] [ITENS]
// Por: Thiago Carvalho - 2025

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>       // nanosleep (usleep eh obsoleto)

#define MAX_QUEUE_SIZE 32  	// capacidade máxima do buffer circular

// Estrutura da fila/buffer circular
typedef struct {
	// informações do produtor
	int* buf;
	size_t cap, head, tail, count;
	bool isClosed;                 // quando true, produtores encerraram
	pthread_mutex_t mtx;
	pthread_cond_t  cv_not_empty;
	pthread_cond_t  cv_not_full;

} BQueue;

// Estrutura dos argumentos do produtor
typedef struct {

	BQueue*		q;		// fila compartilhada

	int			items; 	// quantos itens produzir

} ProducerArgs;

// Estrutura dos argumentos do consumidor
typedef struct {

	BQueue*		q;				// fila compartilhada
	int			id;				// id do consumidor
	long long	partial_sum;	// soma parcial dos itens consumidos

} ConsumerArgs;


// Inicializa a fila
static void bq_init(BQueue* q, size_t cap) {
	q->buf   = (int*)malloc(sizeof(int) * cap);
	q->cap   = cap;
	q->head  = q->tail = q->count = 0;
	q->isClosed = false;

	// inicializa o Mutex que vai proteger a fila
	pthread_mutex_init(&q->mtx, NULL);

	// inicializa as variáveis de condição, usadas para notificar
	// produtores/consumidores sobre mudanças no estado da fila
	pthread_cond_init(&q->cv_not_empty, NULL);
	pthread_cond_init(&q->cv_not_full,  NULL);
}

// Cleanup da fila
static void bq_destroy(BQueue* q) {
	free(q->buf);
	pthread_mutex_destroy(&q->mtx);
	pthread_cond_destroy(&q->cv_not_empty);
	pthread_cond_destroy(&q->cv_not_full);
}

// Enfileira; retorna false se fila já foi fechada
static bool bq_push(BQueue* q, int v) {
	// trava mutex para acessar a fila
	pthread_mutex_lock(&q->mtx);

	// verifica se a fila já foi fechada
	if (q->isClosed) {
		pthread_mutex_unlock(&q->mtx);
		return false;
	}

	// espera até que haja espaço na fila
	while (q->count == q->cap) {
		pthread_cond_wait(&q->cv_not_full, &q->mtx);
		if (q->isClosed) { // re-checa após acordar
			pthread_mutex_unlock(&q->mtx);
			return false;
		}
	}

	q->buf[q->tail] = v;
	q->tail = (q->tail + 1) % q->cap;
	q->count++;
	pthread_cond_signal(&q->cv_not_empty);
	pthread_mutex_unlock(&q->mtx);
	return true;
}

// Desenfileira; retorna false quando (fechada ou vazia)
static bool bq_pop(BQueue* q, int* out) {
	pthread_mutex_lock(&q->mtx);

	// espera até que haja algo na fila
	while (q->count == 0 && !q->isClosed) {
		pthread_cond_wait(&q->cv_not_empty, &q->mtx);
	}

	if (q->count == 0 && q->isClosed) {
		pthread_mutex_unlock(&q->mtx);
		return false;
	}

	// retira o item da fila
	*out = q->buf[q->head];

	// atualiza índices e contadores
	q->head = (q->head + 1) % q->cap;
	q->count--;

	// notifica produtores que há espaço na fila
	pthread_cond_signal(&q->cv_not_full);

	// libera mutex
	pthread_mutex_unlock(&q->mtx);

	return true;
}

// Fecha fila: consumidores drenam e saem
static void bq_close(BQueue* q) {
	pthread_mutex_lock(&q->mtx);
	q->isClosed = true;
	pthread_cond_broadcast(&q->cv_not_empty);
	pthread_cond_broadcast(&q->cv_not_full);
	pthread_mutex_unlock(&q->mtx);
}

static void* producer_thread(void* arg) {
	ProducerArgs*   pa	= (ProducerArgs*)arg;
	struct timespec ts	= {0};

	for (int i = 0; i < pa->items; i++) {
		// simula trabalho do produtor
		ts.tv_sec = 0;
		ts.tv_nsec = 1000000; // 1 ms
		nanosleep(&ts, NULL);

		// produz item i e enfileira, se nao conseguir, sai
		if (!bq_push(pa->q, i)) break;
	}

	bq_close(pa->q);
	return NULL;
}

static void* consumer_thread(void* arg) {
	ConsumerArgs *ca = (ConsumerArgs*)arg;
	int x;
	while (bq_pop(ca->q, &x)) {
		// trabalho do consumidor
		ca->partial_sum += x;
		// usleep(500); // opcional: simular processamento
	}
	return NULL;
}

int main(int argc, char** argv) {
	int				n_items			= 0;		// Número de itens a produzir
	int				n_consumers		= 0;		// Número de consumidores
	BQueue			q				= {0};		// Fila/buffer compartilhado
	pthread_t		prod			= {0};		// Thread do produtor
	ProducerArgs	pa				= {0};		// Argumentos do produtor
	pthread_t*		cons			= {0};		// Vetor de threads dos consumidores
	ConsumerArgs*	cargs			= {0};		// Vetor de argumentos dos consumidores
	int				total 			= 0;		// Soma total dos itens consumidos
	int				expected		= 0;		// Soma esperada

	if (argc > 3) {
		printf("Uso: %s [N_CONSUMIDORES] [ITENS]\n", argv[0]);
		return 1;
	}

	// Processa argumentos da linha de comando
	n_consumers = (argc > 1 ? atoi(argv[1]) : 4);
	n_items		= (argc > 2 ? atoi(argv[2]) : 100);

	if (n_consumers <= 0) {
		printf("Número de consumidores deve ser maior que zero.\n");
		return 1;
	}

	if (n_items < 0) {
		printf("Número de itens deve ser não negativo.\n");
		return 1;
	}

	printf("Iniciando com %d consumidores e %d itens a produzir\n", n_consumers, n_items);

	// Inicializa a fila/buffer compartilhado
	bq_init(&q, MAX_QUEUE_SIZE);

	// Inicializa argumentos do produtor e cria a thread do produtor
	pa.q     = &q;
	pa.items = n_items;
	if (pthread_create(&prod, NULL, producer_thread, &pa) != 0) {
		perror("pthread_create(produtor)");
		return 1;
	}

	// malloc = aloca objeto sem zerar a memória
	cons  = (pthread_t*)malloc(sizeof(pthread_t) * (size_t)n_consumers);

	// calloc = aloca objeto e zera a memória, importante para structs, tbm garante que partial_sum começa em 0
	cargs = (ConsumerArgs*)calloc((size_t)n_consumers, sizeof(ConsumerArgs));

	// Cria threads dos consumidores
	for (int i = 0; i < n_consumers; i++) {
		cargs[i].q           = &q;
		cargs[i].id          = i;
		cargs[i].partial_sum = 0;
		if (pthread_create(&cons[i], NULL, consumer_thread, &cargs[i]) != 0) {
			perror("pthread_create(consumer)");
			return 1;
		}
	}

	// Aguarda o término do produtor
	pthread_join(prod, NULL);

	// Aguarda o término dos consumidores e soma os resultados
	for (int i = 0; i < n_consumers; i++) {
		pthread_join(cons[i], NULL);
		total += cargs[i].partial_sum;
	}

	// Calcula a soma esperada: 0 + 1 + ... + (n_items-1)
	expected = (n_items - 1) * (n_items) / 2;
	printf("consumidores=%d itens=%d soma_total=%d esperado=%d %s\n",
		n_consumers, n_items, total, expected, (total == expected ? "OK" : "MISMATCH"));

	// Libera recursos
	free(cons);
	free(cargs);
	bq_destroy(&q);

	return 0;
}
