/*
 * Hierarquia de memória + localidade de referência
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define N_REAL 4096                     // matriz N x N de int = 64 MB
#define ACESSOS ((size_t)N_REAL * N_REAL)
#define VET_PEQ 1024                    // 4 KB: cabe na cache L1

/* ---------- Parâmetros da Parte 2 (simulação) ---------- */
#define N_SIM 512                       /* matriz 512 x 512 de int = 1 MB */
#define ACESSOS_SIM ((size_t)N_SIM * N_SIM)

#define T1 0.01                         
#define T2 0.10                         

static double agora(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

static uint64_t estado = 88172645463325252ULL;
static uint64_t aleatorio(void) {
    estado ^= estado << 13;
    estado ^= estado >> 7;
    estado ^= estado << 17;
    return estado;
}


static volatile long sorvedouro;   /* evita que o compilador remova os laços */

static double t_linhas(int *m) {
    long s = 0; double t0 = agora();
    for (int i = 0; i < N_REAL; i++)
        for (int j = 0; j < N_REAL; j++)
            s += m[(size_t)i * N_REAL + j];
    double t = agora() - t0; sorvedouro = s; return t;
}

static double t_colunas(int *m) {
    long s = 0; double t0 = agora();
    for (int j = 0; j < N_REAL; j++)
        for (int i = 0; i < N_REAL; i++)
            s += m[(size_t)i * N_REAL + j];
    double t = agora() - t0; sorvedouro = s; return t;
}

static double t_laco_pequeno(int *v) {
    long s = 0; double t0 = agora();
    size_t reps = ACESSOS / VET_PEQ;
    for (size_t r = 0; r < reps; r++)
        for (int i = 0; i < VET_PEQ; i++)
            s += v[i];
    double t = agora() - t0; sorvedouro = s; return t;
}

static double t_aleatorio(int *m) {
    long s = 0; double t0 = agora();
    for (size_t k = 0; k < ACESSOS; k++)
        s += m[aleatorio() % ACESSOS];
    double t = agora() - t0; sorvedouro = s; return t;
}


typedef struct {
    size_t n_linhas, tam_linha;
    int64_t *tag;           
    long acertos, erros;
} Cache;

static void cache_init(Cache *c, size_t tam_cache, size_t tam_linha) {
    c->tam_linha = tam_linha;
    c->n_linhas = tam_cache / tam_linha;
    c->tag = malloc(c->n_linhas * sizeof(int64_t));
    for (size_t i = 0; i < c->n_linhas; i++) c->tag[i] = -1;
    c->acertos = c->erros = 0;
}

static void cache_acessa(Cache *c, uint64_t endereco) {
    uint64_t bloco = endereco / c->tam_linha;
    size_t linha = bloco % c->n_linhas;
    int64_t tag = bloco / c->n_linhas;
    if (c->tag[linha] == tag) c->acertos++;
    else { c->erros++; c->tag[linha] = tag; }
}

static void relatorio(const char *nome, const char *esp, const char *tmp,
                      Cache *c) {
    long total = c->acertos + c->erros;
    double H = (double)c->acertos / total;
    double Ts = H * T1 + (1.0 - H) * (T1 + T2);
    printf("%-18s %-6s %-6s  H=%6.2f%%  miss=%6.2f%%  Ts=%.4f us\n",
           nome, esp, tmp, 100.0 * H, 100.0 * (1.0 - H), Ts);
}

int main(int argc, char **argv) {
    size_t tam_cache = argc > 1 ? strtoul(argv[1], NULL, 10) : 32768;
    size_t tam_linha = argc > 2 ? strtoul(argv[2], NULL, 10) : 64;
    const size_t B = sizeof(int);

    int *m = malloc(ACESSOS * sizeof(int));
    if (!m) { fprintf(stderr, "sem memoria\n"); return 1; }
    for (size_t i = 0; i < ACESSOS; i++) m[i] = (int)(i & 7);

    printf("=== PARTE 1: tempo real (%zu acessos por padrao) ===\n", ACESSOS);
    printf("%-18s %10.1f ms\n", "Por linhas",   t_linhas(m));
    printf("%-18s %10.1f ms\n", "Por colunas",  t_colunas(m));
    printf("%-18s %10.1f ms\n", "Laco pequeno", t_laco_pequeno(m));
    printf("%-18s %10.1f ms\n", "Aleatorio",    t_aleatorio(m));
    free(m);

    /* ---------- Parte 2 ---------- */
    printf("\n=== PARTE 2: simulacao (cache %zu B, linha %zu B, mapeamento direto) ===\n",
           tam_cache, tam_linha);
    printf("%zu acessos por padrao; T1=%.2f us, T2=%.2f us\n\n",
           ACESSOS_SIM, T1, T2);
    printf("%-18s %-6s %-6s  %-9s %-11s %s\n",
           "Padrao", "Espac.", "Temp.", "Acerto", "Erro", "Tempo medio");

    Cache c;

    cache_init(&c, tam_cache, tam_linha);              /* por linhas */
    for (int i = 0; i < N_SIM; i++)
        for (int j = 0; j < N_SIM; j++)
            cache_acessa(&c, ((uint64_t)i * N_SIM + j) * B);
    relatorio("Por linhas", "alta", "baixa", &c); free(c.tag);

    cache_init(&c, tam_cache, tam_linha);              /* por colunas */
    for (int j = 0; j < N_SIM; j++)
        for (int i = 0; i < N_SIM; i++)
            cache_acessa(&c, ((uint64_t)i * N_SIM + j) * B);
    relatorio("Por colunas", "baixa", "baixa", &c); free(c.tag);

    cache_init(&c, tam_cache, tam_linha);              
    for (size_t r = 0; r < ACESSOS_SIM / VET_PEQ; r++)
        for (int i = 0; i < VET_PEQ; i++)
            cache_acessa(&c, (uint64_t)i * B);
    relatorio("Laco pequeno", "alta", "alta", &c); free(c.tag);

    cache_init(&c, tam_cache, tam_linha);             
    for (size_t k = 0; k < ACESSOS_SIM; k++)
        cache_acessa(&c, (aleatorio() % ACESSOS_SIM) * B);
    relatorio("Aleatorio", "baixa", "baixa", &c); free(c.tag);

    return 0;
}
