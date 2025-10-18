// Game of Life sequencial
// Uso: ./game_of_life LARG ALT PASSOS DENSIDADE
// Por: Thiago Carvalho - 2025

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

typedef struct {

  int		width;		// largura
  int		height;		// altura
  uint8_t*	curr;		// grade atual (0 morto, 1 vivo)
  uint8_t*	next;		// próxima grade

} Grid;

// inicia a grade com valor 1 (vivo) com chance = densidade
static void init_grid(Grid* g, int width, int height, double dens) {
	int				total	= 0;
	int				i		= 0;
	unsigned int 	seed	= 1234;

	g->width 	= width;
	g->height 	= height;
	total 		= width * height;

	g->curr 	= (uint8_t*)malloc((size_t)total);
	g->next 	= (uint8_t*)malloc((size_t)total);

	if (!g->curr || !g->next) {
		fprintf(stderr, "falha na alocacao de memoria\n");
		exit(1);
	}

	// Inicializa o gerador de números aleatórios
	if (seed == 0) {
        srand((unsigned int)time(NULL));
    } else {
        srand(seed);
    }

	for (int i = 0; i < total; i++) {
        // Gera um número inteiro aleatório entre 0 e RAND_MAX
        int r_val = rand(); 

        // Normaliza o número aleatório para o intervalo [0.0, 1.0]
        double normalized_r = (double)r_val / (double)RAND_MAX;

        // Verifica a densidade: se o número aleatório for menor que a densidade
        if (normalized_r < dens) {
            g->curr[i] = 1; // Vivo
        } else {
            g->curr[i] = 0; // Morto
        }
    }

	memset(g->next, 0, (size_t)total);
}

// libera memoria
static void free_grid(Grid* g) {
  free(g->curr);
  free(g->next);
  g->curr = NULL;
  g->next = NULL;
}

// Conta vizinhos vivos (8 vizinhos), fora da borda conta como 0
static inline int count_neighbors(const Grid* g, int x, int y) {
	int w 	= g->width;
	int h 	= g->height;
	int sum = 0;

	// Percorre os vizinhos ao redor da célula (x, y)
	for (int dy = -1; dy <= 1; dy++) {
		for (int dx = -1; dx <= 1; dx++) {
			// Ignora a própria célula
			if (dx == 0 && dy == 0) continue;

			int nx = x + dx;
			int ny = y + dy;

			// Verifica se está dentro dos limites da grade
			if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
				sum += g->curr[ny * w + nx];
			}
		}
	}
	return sum;
}

// faz um passo sequencial
static void step_seq(Grid* g) {
	int			w	= 0;
	int			h	= 0;
	int			x	= 0;
	int			y	= 0;
	int			n	= 0;
	uint8_t*	src	= NULL;
	uint8_t*	dst	= NULL;

	w 	= g->width;
	h 	= g->height;
	src = g->curr;
	dst = g->next;

	// percorre todas as células chamando count_neighbors para cada uma (sequencial)
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			n = count_neighbors(g, x, y);
			if (src[y * w + x]) {
				// viva com 2 ou 3 vizinhos permanece viva
				// menos de 2 ou mais de 3 morre
				dst[y * w + x] = (n == 2 || n == 3) ? 1u : 0u;
			} else {
				// morta com 3 vizinhos nasce
				dst[y * w + x] = (n == 3) ? 1u : 0u;
			}
		}
	}

	// no final, troca os buffers para o próximo passo para não precisar copiar dados
	g->curr = dst;
	g->next = src;
}

// roda varios passos no modo pedido (0=seq, 1=paralelo)
static double run_steps(Grid* g, int steps) {
	int			s		= 0;
	double		t0		= 0.0;
	double		t1		= 0.0;

	t0 = omp_get_wtime();

	for (s = 0; s < steps; s++) {
		step_seq(g);
	}

	t1 = omp_get_wtime();

	return t1 - t0;
}

// conta celulas vivas (para checar resultado)
static long long count_alive(const Grid* g) {
	int			i		= 0;
	int			total	= 0;
	long long	sum		= 0;

	total = g->width * g->height;
	sum = 0;
	for (i = 0; i < total; i++) {
		sum += g->curr[i] ? 1 : 0;
	}
	return sum;
}

int main(int argc, char** argv) {
	int			width		= 100;
	int			height		= 100;
	int			steps		= 1000;
	double		dens		= 0.5;
	Grid		g			= {0};
	double		time_seq	= 0.0;
	long long	pop0		= 0;
	long long	pop_end		= 0;

	if (argc > 5) {
		printf("Uso: %s LARG ALT PASSOS DENSIDADE\n", argv[0]);
		return 1;
	}

	if (argc > 1) width		= atoi(argv[1]);
	if (argc > 2) height	= atoi(argv[2]);
	if (argc > 3) steps		= atoi(argv[3]);
	if (argc > 4) dens		= atof(argv[4]);

	if (width <= 0 || height <= 0 || steps < 0 || dens < 0.0 || dens > 1.0) {
		printf("parametros invalidos\n");
		return 1;
	}

	init_grid(&g, width, height, dens);

	pop0 = count_alive(&g);

	time_seq = run_steps(&g, steps);
	pop_end = count_alive(&g);

	printf("Sequencial:\n");
	printf("  Tamanho: %dx%d\n"			, width, height);
	printf("  Passos: %d\n"				, steps);
	printf("  Densidade inicial: %.3f\n"	, dens);
	printf("  Vivos inicio: %lld\n"		, pop0);
	printf("  Vivos fim: %lld\n"			, pop_end);
	printf("  Tempo: %.6f s\n"			, time_seq);

	free_grid(&g);
	return 0;
}
