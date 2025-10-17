#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_QUEUE_SIZE 32

typedef struct {
	int buf[MAX_QUEUE_SIZE];
	int head, tail, count;
	int isClosed;
} BQueue;

void bq_init(BQueue* q) {
	q->head = q->tail = q->count = 0;
	q->isClosed = 0;
}

int bq_push(BQueue* q, int v) {
	int ok = 0;
	#pragma omp critical
	{
		if (!q->isClosed && q->count < MAX_QUEUE_SIZE) {
			q->buf[q->tail] = v;
			q->tail = (q->tail + 1) % MAX_QUEUE_SIZE;
			q->count++;
			ok = 1;
		}
	}
	return ok;
}

int bq_pop(BQueue* q, int* out) {
	int ok = 0;
	#pragma omp critical
	{
		if (q->count > 0) {
			*out = q->buf[q->head];
			q->head = (q->head + 1) % MAX_QUEUE_SIZE;
			q->count--;
			ok = 1;
		} else if (q->isClosed) {
			ok = -1;
		}
	}
	return ok;
}

void bq_close(BQueue* q) {
	#pragma omp critical
	{
		q->isClosed = 1;
	}
}

int main(int argc, char** argv) {
	int n_consumers = (argc > 1 ? atoi(argv[1]) : 4);
	int n_items = (argc > 2 ? atoi(argv[2]) : 100);
	BQueue q;
	bq_init(&q);

	int* partial_sum = calloc(n_consumers, sizeof(int));

	#pragma omp parallel num_threads(n_consumers + 1) shared(q, partial_sum)
	{
		int tid = omp_get_thread_num();
		if (tid == 0) {
			// Produtor
			for (int i = 0; i < n_items; i++) {
				while (!bq_push(&q, i)) {
					#pragma omp flush
				}

				// Simular trabalho do produtor
				#pragma omp critical
				{
					// Seção crítica para simular trabalho
					for (volatile int j = 0; j < 10000; j++);
				}
			}
			bq_close(&q);
		} else {
			// Consumidor
			int x;
			while (1) {
				int res = bq_pop(&q, &x);
				if (res == 1) {
					partial_sum[tid - 1] += x;
				} else if (res == -1) {
					break;
				}

				// Simular trabalho do consumidor
				#pragma omp critical
				{
					// Seção crítica para simular trabalho
					for (volatile int j = 0; j < 10000; j++);
				}
			}
		}
	}

	int total = 0;
	for (int i = 0; i < n_consumers; i++)
		total += partial_sum[i];

	int expected = (n_items - 1) * n_items / 2;
	printf("consumidores=%d itens=%d soma_total=%d esperado=%d %s\n",
		n_consumers, n_items, total, expected, (total == expected ? "OK" : "MISMATCH"));

	free(partial_sum);
	return 0;
}
