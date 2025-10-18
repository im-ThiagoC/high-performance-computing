// Game of Life paralelo com OpenMP
// Uso: ./game_of_life_omp LARG ALT PASSOS DENSIDADE [MODO] [THREADS]
// Exemplo: ./game_of_life_omp 100 100 1000 0.5 2 4
// MODO: 0=sequencial, 1=paralelo, 2=ambos (mede speedup)
// Por: Thiago Carvalho - 2025

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <omp.h>

typedef struct {

	int			width;		// largura
	int			height;		// altura
	uint8_t*	curr;		// grade atual (0 morto, 1 vivo)
	uint8_t*	next;		// próxima grade

} Grid;

// inicia a grade com valor 1 (vivo) com chance = densidade
static void init_grid(Grid* g, int width, int height, double dens, unsigned int seed) {
	int				total	= 0;
	int				i		= 0;

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

// faz um passo paralelo (OpenMP)
static void step_omp(Grid* g) {
	int				w			= 0;
	int				h			= 0;
	int				x			= 0;
	int				y			= 0;
	int				n			= 0;

	// ponteiros para as grades
	uint8_t*	src		= NULL;
	uint8_t*	dst		= NULL;

	// corpo
	w 	= g->width;
	h 	= g->height;
	src = g->curr;
	dst = g->next;

	// percorre todas as células chamando count_neighbors para cada uma (paralelo)
	#pragma omp parallel for private(x,n) schedule(static)
	for (y = 0; y < h; y++) 
	{
		for (x = 0; x < w; x++) 
		{
			n = count_neighbors(g, x, y);
			if (src[y * w + x]) {
				dst[y * w + x] = (n == 2 || n == 3) ? 1u : 0u;
			} else {
				dst[y * w + x] = (n == 3) ? 1u : 0u;
			}
		}
	}

	// troca os buffers
	g->curr = dst;
	g->next = src;
}

// roda varios passos no modo pedido (0=seq, 1=paralelo)
static double run_steps(Grid* g, int steps, int mode) {
	int			s		= 0;
	double		t0		= 0.0;
	double		t1		= 0.0;

	t0 = omp_get_wtime();

	if (mode == 0) {
		for (s = 0; s < steps; s++) {
			step_seq(g);
		}
	} else {
		for (s = 0; s < steps; s++) {
			step_omp(g);
		}
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
	int				width			= 0;		// largura da grade
	int				height			= 0;		// altura da grade
	int				steps			= 0;		// número de passos a executar
	double			dens			= 0.0;		// densidade inicial de células vivas
	int				mode			= 0;		// modo de execução (0=seq, 1=paralelo, 2=ambos)
	int				threads			= 0;		// número de threads (se >0)
	Grid			g_seq			= {0};		// grade para execução sequencial
	Grid			g_par		 	= {0};		// grade para execução paralela
	double			time_seq		= 0.0;		// tempo da execução sequencial
	double			time_par		= 0.0;		// tempo da execução paralela
	double			speedup			= 0.0;		// speedup obtido (seq/par)
	double			eff				= 0.0;		// eficiência paralela
	long long		pop0			= 0;		// população inicial de células vivas
	long long		pop_end			= 0;		// população final de células vivas
	int				use_both		= 0;		// flag para executar ambos os modos
	unsigned int	seed			= 0;		// semente para o gerador de números aleatórios

	// corpo
	if (argc > 5) {
		printf("Uso: %s LARG ALT PASSOS DENSIDADE [MODO] [THREADS]\n", argv[0]);
		printf("MODO: 0=sequencial, 1=paralelo, 2=ambos\n");
		return 1;
	}
	// Valores default
	width 	= 100;
	height 	= 100;
	steps 	= 1000;
	dens 	= 0.5;
	mode 	= 2;
	threads = 0;

	// Lê argumentos se fornecidos
	if (argc > 1) width		= atoi(argv[1]);
	if (argc > 2) height	= atoi(argv[2]);
	if (argc > 3) steps		= atoi(argv[3]);
	if (argc > 4) dens		= atof(argv[4]);
	if (argc > 5) mode		= atoi(argv[5]);
	if (argc > 6) threads	= atoi(argv[6]);

	if (width <= 0 || height <= 0 || steps < 0 || dens < 0.0 || dens > 1.0) {
		printf("parametros invalidos\n");
		return 1;
	}

	if (threads > 0) {
		omp_set_num_threads(threads);
	}

	// Mesma semente por agora
	seed = 123123;

	// inicia grades iguais para comparar (quando usar ambos)
	init_grid(&g_seq, width, height, dens, seed);
	init_grid(&g_par, width, height, dens, seed);

	// Conta a população inicial de células vivas
	pop0 = count_alive(&g_seq);

	// Verifica se deve rodar ambos os modos (sequencial e paralelo)
	use_both = (mode == 2);

	if (mode == 0) {
		// Executa apenas o modo sequencial
		time_seq = run_steps(&g_seq, steps, 0);
		pop_end = count_alive(&g_seq);
		printf("Sequencial:\n");
		printf("  Tamanho: %dx%d\n", width, height);
		printf("  Passos: %d\n", steps);
		printf("  Densidade inicial: %.3f\n", dens);
		printf("  Vivos inicio: %lld\n", pop0);
		printf("  Vivos fim: %lld\n", pop_end);
		printf("  Tempo: %.6f s\n", time_seq);
	} else if (mode == 1) {
		// Executa apenas o modo paralelo
		time_par = run_steps(&g_par, steps, 1);
		pop_end = count_alive(&g_par);
		printf("Paralelo:\n");
		printf("  Tamanho: %dx%d\n", width, height);
		printf("  Passos: %d\n", steps);
		printf("  Densidade inicial: %.3f\n", dens);
		printf("  Threads: %d\n", (threads > 0 ? threads : omp_get_max_threads()));
		printf("  Vivos inicio: %lld\n", pop0);
		printf("  Vivos fim: %lld\n", pop_end);
		printf("  Tempo: %.6f s\n", time_par);
	} else {
		// Executa ambos os modos para comparar desempenho
		time_seq = run_steps(&g_seq, steps, 0);
		time_par = run_steps(&g_par, steps, 1);
		speedup = time_seq / time_par;
		eff = speedup / (double)(threads > 0 ? threads : omp_get_max_threads());
		pop_end = count_alive(&g_par);
		printf("Comparacaoo Sequencial x Paralelo:\n");
		printf("  Tamanho: %dx%d\n", width, height);
		printf("  Passos: %d\n", steps);
		printf("  Densidade inicial: %.3f\n", dens);
		printf("  Threads: %d\n", (threads > 0 ? threads : omp_get_max_threads()));
		printf("  Tempo sequencial: %.6f s\n", time_seq);
		printf("  Tempo paralelo:   %.6f s\n", time_par);
		printf("  Speedup: %.3f\n", speedup);
		printf("  Eficiencia: %.3f\n", eff);
		printf("  Vivos inicio: %lld\n", pop0);
		printf("  Vivos fim: %lld\n", pop_end);
	}

	free_grid(&g_seq);
	free_grid(&g_par);
	return 0;
}
