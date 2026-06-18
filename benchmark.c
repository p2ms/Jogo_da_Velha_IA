/* 
   programa de avaliação dos três algoritmos
   aqui a gente faz duas coisas principais:
   - parte 1: mede o tempo médio e os nós explorados por findBestMove,
     variando o número de marcações iniciais de 1 a 4. repete com 5 seeds
     diferentes pra ter resultados mais confiáveis.
   - parte 2: torneio cruzado entre os algoritmos, onde cada par joga
     1000 partidas a partir de posições aleatórias e a gente conta as vitórias.

   o stdout é redirecionado pra /dev/null pra silenciar o printf do minimax original sem precisar modificar o código da aula. toda saída vai pro stderr. */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "board.h"
#include "minimax.h"
#include "alphabeta.h"
#include "astar.h"
#include "nodes.h"

// parâmetros do experimento 
#define NUM_BOARDS      50  // posições aleatórias distintas por nível de marcação 
#define REPS_FAST      500 // repetições por posição pra alfa-beta e A* 
#define REPS_SLOW       50  // repetições por posição pra minimax (mais lento) 
#define GAMES_PER_PAIR 20  // partidas por par de algoritmos no torneio 
#define NUM_SEEDS        5  // seeds diferentes pra calcular média e desvio 

// seeds usadas: escolhemos valores variados pra garantir diversidade nas posições 
static const unsigned int SEEDS[NUM_SEEDS] = { 42, 7, 123, 2024, 999 };

// enum pra identificar os algoritmos sem usar números mágicos no código
typedef enum { ALG_MINIMAX, ALG_ALPHABETA, ALG_ASTAR } Algorithm;
static const char *ALG_NAMES[3] = { "Minimax", "AlfaBeta", "AStar" };

// despacha a chamada de findBestMove pro algoritmo correto. assim o torneio e o benchmark não precisam de if/else por todo lado. 
static move findBestMoveBy(Algorithm a, char board[3][3], bool isMax) {
    switch (a) {
        case ALG_MINIMAX:   return findBestMove(board, isMax);
        case ALG_ALPHABETA: return findBestMoveAB(board, isMax);
        case ALG_ASTAR:     return findBestMoveAStar(board, isMax);
    }
    move m = { -1, -1 };
    return m;
}

// preenche o tabuleiro com '_' em todas as posições 
static void clearBoard(char b[3][3]) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            b[i][j] = '_';
}

// copia o conteúdo de src pra dst (9 bytes = 9 células do tabuleiro 3x3)
static void copyBoard(char dst[3][3], char src[3][3]) {
    memcpy(dst, src, 9);
}

// gera um tabuleiro aleatório com n_marks marcações. as marcações são colocadas alternando entre 'x' e 'o' (x começa), em posições aleatórias. se o tabuleiro gerado já tiver um vencedor, descarta e tenta de novo até gerar uma posição válida. 
static void genRandomBoard(char board[3][3], int n_marks) {
    while (1) {
        clearBoard(board);
        int placed = 0;
        while (placed < n_marks) {
            int r = rand() % 3;
            int c = rand() % 3;
            if (board[r][c] == '_') {
                board[r][c] = (placed % 2 == 0) ? 'x' : 'o';
                placed++;
            }
        }
        // só aceita posições sem vencedor (utility == 0) 
        if (utility(board) == 0) return;
    }
}

// retorna o tempo atual em segundos com alta resolução. usamos CLOCK_MONOTONIC pra não ser afetado por ajustes do relógio do sistema. 
static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* mede o tempo médio e os nós explorados por chamada de findBestMove.
   pra cada uma das num_boards posições aleatórias:
     - tempo: média de 'reps' chamadas (repetições pra estabilizar a medição)
     - nós: uma única chamada (o resultado é determinístico, não muda)
   retorna a média do tempo entre posições.
   armazena o desvio padrão do tempo em *sd_time,
   e a média e desvio dos nós em *mean_nodes e *sd_nodes. */
static double benchAlgorithmFull(Algorithm a, int n_marks, int num_boards, int reps,
                                  double *sd_time,
                                  double *mean_nodes, double *sd_nodes) {
    char board[3][3], scratch[3][3];
    if (num_boards > NUM_BOARDS) num_boards = NUM_BOARDS;

    double time_samples[NUM_BOARDS]; // tempo médio de cada posição 
    double node_samples[NUM_BOARDS]; // nós explorados em cada posição 

    for (int b = 0; b < num_boards; b++) {
        genRandomBoard(board, n_marks);

        // quem joga a seguir: x se n_marks for par (x sempre começa), o se n_marks for ímpar 
        bool isMax = (n_marks % 2 == 0);

        // medição de tempo: repete 'reps' vezes a mesma chamada pra estabilizar 
        double t0 = now_seconds();
        for (int r = 0; r < reps; r++) {
            copyBoard(scratch, board); // copia pra não modificar o tabuleiro original 
            reset_nodes();
            (void)findBestMoveBy(a, scratch, isMax);
        }
        double t1 = now_seconds();
        time_samples[b] = (t1 - t0) / reps; // tempo médio por chamada

        // contagem de nós: uma única chamada (resultado é sempre igual) 
        copyBoard(scratch, board);
        reset_nodes();
        (void)findBestMoveBy(a, scratch, isMax);
        node_samples[b] = (double)get_nodes();
    }

    // calcula média e desvio padrão amostral do tempo
    double mean_t = 0.0;
    for (int b = 0; b < num_boards; b++) mean_t += time_samples[b];
    mean_t /= num_boards;
    double var_t = 0.0;
    for (int b = 0; b < num_boards; b++) {
        double d = time_samples[b] - mean_t;
        var_t += d * d;
    }
    *sd_time = (num_boards > 1) ? sqrt(var_t / (num_boards - 1)) : 0.0;

    // calcula média e desvio padrão amostral dos nós 
    double mn = 0.0;
    for (int b = 0; b < num_boards; b++) mn += node_samples[b];
    mn /= num_boards;
    double var_n = 0.0;
    for (int b = 0; b < num_boards; b++) {
        double d = node_samples[b] - mn;
        var_n += d * d;
    }
    *mean_nodes = mn;
    *sd_nodes   = (num_boards > 1) ? sqrt(var_n / (num_boards - 1)) : 0.0;

    return mean_t;
}

/* simula uma partida completa entre dois algoritmos.
   algA_x usa o marcador 'x' e começa; algB_o usa 'o'.
   a vez de quem joga é determinada pela paridade de initial_marks:
   se par, é a vez do x; se ímpar, é a vez do o.
   retorna +10 se x venceu, -10 se o venceu, 0 se empatou. */
static int playGame(Algorithm algA_x, Algorithm algB_o,
                    char initial[3][3], int initial_marks) {
    char board[3][3];
    copyBoard(board, initial);
    bool xTurn = (initial_marks % 2 == 0);

    while (utility(board) == 0 && isMovesLeft(board)) {
        move m;
        if (xTurn) {
            m = findBestMoveBy(algA_x, board, true);
            if (m.row < 0) break; // sem movimento válido, encerra
            play(board, m, 'x');
        } else {
            m = findBestMoveBy(algB_o, board, false);
            if (m.row < 0) break;
            play(board, m, 'o');
        }
        xTurn = !xTurn;
    }
    return utility(board);
}

// tenta ler o modelo do processador via sysctl (macos) ou /proc/cpuinfo (linux). imprime no arquivo de saída indicado.
static void printCPU(FILE *out) {
    FILE *p = popen("sysctl -n machdep.cpu.brand_string 2>/dev/null", "r");
    char buf[256] = {0};
    if (p) {
        if (fgets(buf, sizeof(buf), p) == NULL) buf[0] = '\0';
        pclose(p);
    }
    // fallback pra linux caso o sysctl não retorne nada 
    if (buf[0] == '\0') {
        FILE *f = fopen("/proc/cpuinfo", "r");
        if (f) {
            char line[256];
            while (fgets(line, sizeof(line), f)) {
                if (strncmp(line, "model name", 10) == 0) {
                    char *colon = strchr(line, ':');
                    if (colon) strncpy(buf, colon + 2, sizeof(buf) - 1);
                    break;
                }
            }
            fclose(f);
        }
    }
    if (buf[0] == '\0') strcpy(buf, "desconhecido\n");
    fprintf(out, "cpu: %s", buf);
}

int main(void) {
    // redireciona stdout pro lixo pra silenciar o printf do minimax original. toda saída do benchmark vai pro stderr. 
    fflush(stdout);
    freopen("/dev/null", "w", stdout);
    FILE *out = stderr;

    // cabeçalho com os parâmetros usados no experimento 
    fprintf(out, "===========================================================\n");
    fprintf(out, " trabalho 1 ia — minimax x poda alfa-beta x a*\n");
    printCPU(out);
    fprintf(out, " num_boards=%d  reps_fast=%d  reps_slow=%d\n",
            NUM_BOARDS, REPS_FAST, REPS_SLOW);
    fprintf(out, " games_per_pair=%d  num_seeds=%d  seeds=",
            GAMES_PER_PAIR, NUM_SEEDS);
    for (int s = 0; s < NUM_SEEDS; s++)
        fprintf(out, "%u%s", SEEDS[s], s < NUM_SEEDS - 1 ? "," : "\n");
    fprintf(out, "===========================================================\n\n");

    /* ------------------------------------------------------------------ */
    /* parte 1: tempo e nós explorados por algoritmo × marcações iniciais */
    /* ------------------------------------------------------------------ */

    // armazena a média de tempo e nós pra cada combinação de (marcações, algoritmo, seed). depois consolidamos entre seeds. 
    double sm_time [4][3][NUM_SEEDS];
    double sm_nodes[4][3][NUM_SEEDS];

    fprintf(out, "[parte 1] coletando tempos e nos explorados (%d seeds x %d posicoes)...\n",
            NUM_SEEDS, NUM_BOARDS);

    // roda o benchmark pra cada seed 
    for (int si = 0; si < NUM_SEEDS; si++) {
        srand(SEEDS[si]);

        // pra cada número de marcações iniciais (1 a 4) 
        for (int nm = 1; nm <= 4; nm++) {
            double sd_t, mn, sd_n;

            // mede minimax com menos repetições (é mais lento) 
            sm_time[nm-1][ALG_MINIMAX][si] =
                benchAlgorithmFull(ALG_MINIMAX, nm, NUM_BOARDS, REPS_SLOW, &sd_t, &mn, &sd_n);
            sm_nodes[nm-1][ALG_MINIMAX][si] = mn;

            // mede alfa-beta 
            sm_time[nm-1][ALG_ALPHABETA][si] =
                benchAlgorithmFull(ALG_ALPHABETA, nm, NUM_BOARDS, REPS_FAST, &sd_t, &mn, &sd_n);
            sm_nodes[nm-1][ALG_ALPHABETA][si] = mn;

            // mede a* 
            sm_time[nm-1][ALG_ASTAR][si] =
                benchAlgorithmFull(ALG_ASTAR, nm, NUM_BOARDS, REPS_FAST, &sd_t, &mn, &sd_n);
            sm_nodes[nm-1][ALG_ASTAR][si] = mn;
        }
        fprintf(out, "  seed %u concluida.\n", SEEDS[si]);
    }

    // consolida os resultados: calcula média e desvio padrão entre as 5 seeds 
    double grand_mean_t[4][3], grand_sd_t[4][3];
    double grand_mean_n[4][3], grand_sd_n[4][3];

    for (int nm = 1; nm <= 4; nm++) {
        for (int alg = 0; alg < 3; alg++) {
            double mt = 0.0, mn = 0.0;
            for (int si = 0; si < NUM_SEEDS; si++) {
                mt += sm_time[nm-1][alg][si];
                mn += sm_nodes[nm-1][alg][si];
            }
            mt /= NUM_SEEDS;
            mn /= NUM_SEEDS;
            grand_mean_t[nm-1][alg] = mt;
            grand_mean_n[nm-1][alg] = mn;

            double vt = 0.0, vn = 0.0;
            for (int si = 0; si < NUM_SEEDS; si++) {
                double dt = sm_time[nm-1][alg][si]  - mt;
                double dn = sm_nodes[nm-1][alg][si] - mn;
                vt += dt * dt;
                vn += dn * dn;
            }
            grand_sd_t[nm-1][alg] = (NUM_SEEDS > 1) ? sqrt(vt / (NUM_SEEDS - 1)) : 0.0;
            grand_sd_n[nm-1][alg] = (NUM_SEEDS > 1) ? sqrt(vn / (NUM_SEEDS - 1)) : 0.0;
        }
    }

    // tabela de tempo médio por chamada 
    fprintf(out, "\n[parte 1a] tempo medio por chamada de findbestmove (segundos)\n");
    fprintf(out, "formato: media +/- desvio padrao entre seeds\n\n");
    fprintf(out, "%-6s | %-28s | %-28s | %-28s\n", "marcas", "minimax", "alfabeta", "astar");
    fprintf(out, "-------+------------------------------+------------------------------+------------------------------\n");
    for (int nm = 1; nm <= 4; nm++) {
        fprintf(out, "%-6d | %10.3e  +/-  %9.2e  | %10.3e  +/-  %9.2e  | %10.3e  +/-  %9.2e\n",
                nm,
                grand_mean_t[nm-1][0], grand_sd_t[nm-1][0],
                grand_mean_t[nm-1][1], grand_sd_t[nm-1][1],
                grand_mean_t[nm-1][2], grand_sd_t[nm-1][2]);
    }

    // tabela de nós explorados por chamada 
    fprintf(out, "\n[parte 1b] nos explorados por chamada de findbestmove\n");
    fprintf(out, "formato: media +/- desvio padrao entre seeds\n\n");
    fprintf(out, "%-6s | %-28s | %-28s | %-28s\n", "marcas", "minimax", "alfabeta", "astar");
    fprintf(out, "-------+------------------------------+------------------------------+------------------------------\n");
    for (int nm = 1; nm <= 4; nm++) {
        fprintf(out, "%-6d | %10.1f  +/-  %9.1f  | %10.1f  +/-  %9.1f  | %10.1f  +/-  %9.1f\n",
                nm,
                grand_mean_n[nm-1][0], grand_sd_n[nm-1][0],
                grand_mean_n[nm-1][1], grand_sd_n[nm-1][1],
                grand_mean_n[nm-1][2], grand_sd_n[nm-1][2]);
    }

    // tabela de speedup relativo ao minimax 
    fprintf(out, "\n[parte 1c] speedup e reducao de nos em relacao ao minimax\n");
    fprintf(out, "%-6s | %-22s | %-22s\n", "marcas", "alfabeta vs minimax", "astar vs minimax");
    fprintf(out, "-------+------------------------+------------------------\n");
    for (int nm = 1; nm <= 4; nm++) {
        double sp_t_ab = grand_mean_t[nm-1][0] / grand_mean_t[nm-1][1];
        double sp_t_as = grand_mean_t[nm-1][0] / grand_mean_t[nm-1][2];
        double sp_n_ab = grand_mean_n[nm-1][0] / grand_mean_n[nm-1][1];
        double sp_n_as = grand_mean_n[nm-1][0] / grand_mean_n[nm-1][2];
        fprintf(out, "%-6d | tempo %5.1fx  nos %5.1fx  | tempo %5.1fx  nos %5.1fx\n",
                nm, sp_t_ab, sp_n_ab, sp_t_as, sp_n_as);
    }

    // criamos um csv da parte 1 pra ajudar na documentação do relatorio
    fprintf(out, "\ncsv (parte 1):\n");
    fprintf(out, "marcas,mm_time,mm_time_sd,mm_nodes,mm_nodes_sd,"
                 "ab_time,ab_time_sd,ab_nodes,ab_nodes_sd,"
                 "as_time,as_time_sd,as_nodes,as_nodes_sd\n");
    for (int nm = 1; nm <= 4; nm++) {
        fprintf(out, "%d,%.6e,%.6e,%.1f,%.1f,%.6e,%.6e,%.1f,%.1f,%.6e,%.6e,%.1f,%.1f\n",
                nm,
                grand_mean_t[nm-1][0], grand_sd_t[nm-1][0],
                grand_mean_n[nm-1][0], grand_sd_n[nm-1][0],
                grand_mean_t[nm-1][1], grand_sd_t[nm-1][1],
                grand_mean_n[nm-1][1], grand_sd_n[nm-1][1],
                grand_mean_t[nm-1][2], grand_sd_t[nm-1][2],
                grand_mean_n[nm-1][2], grand_sd_n[nm-1][2]);
    }

    /* ------------------------------------------------------------------ */
    /* parte 2: torneio cruzado entre os algoritmos                        */
    /* ------------------------------------------------------------------ */
    fprintf(out, "\n[parte 2] torneio cruzado (%d partidas/par x %d seeds = %d total/par)\n",
            GAMES_PER_PAIR, NUM_SEEDS, GAMES_PER_PAIR * NUM_SEEDS);
    fprintf(out, "posicoes iniciais aleatorias com 1 ou 2 marcacoes.\n\n");

    Algorithm algs[3] = { ALG_MINIMAX, ALG_ALPHABETA, ALG_ASTAR };

    // acumula os resultados de todas as seeds 
    int total_xw[3][3] = {{0}}; // vitórias de x por par (a, b) 
    int total_ow[3][3] = {{0}}; // vitórias de o por par (a, b) 
    int total_dr[3][3] = {{0}}; // empates por par (a, b) 

    for (int si = 0; si < NUM_SEEDS; si++) {
        srand(SEEDS[si]);

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                for (int g = 0; g < GAMES_PER_PAIR; g++) {
                    // alterna entre 1 e 2 marcações iniciais pra ter variabilidade 
                    int nm = 1 + (g % 2);
                    char init[3][3];
                    genRandomBoard(init, nm);
                    int r = playGame(algs[a], algs[b], init, nm);
                    if      (r ==  10) total_xw[a][b]++;
                    else if (r == -10) total_ow[a][b]++;
                    else               total_dr[a][b]++;
                }
            }
        }
        fprintf(out, "  seed %u concluida.\n", SEEDS[si]);
    }

    int total_games = GAMES_PER_PAIR * NUM_SEEDS;

    // tabela detalhada por par de algoritmos 
    fprintf(out, "\n%-12s vs %-12s | %6s %6s %6s | %8s %8s %8s\n",
            "x (alga)", "o (algb)", "x_win", "o_win", "draw", "x_win%", "o_win%", "draw%");
    fprintf(out, "----------------------------+----------------------+----------------------------\n");
    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            int xw = total_xw[a][b], ow = total_ow[a][b], dr = total_dr[a][b];
            fprintf(out, "%-12s vs %-12s | %6d %6d %6d | %7.1f%% %7.1f%% %7.1f%%\n",
                    ALG_NAMES[algs[a]], ALG_NAMES[algs[b]],
                    xw, ow, dr,
                    100.0 * xw / total_games,
                    100.0 * ow / total_games,
                    100.0 * dr / total_games);
        }
    }

    // totais por algoritmo (somando partidas como x e como o) 
    int wins_alg[3]   = {0};
    int losses_alg[3] = {0};
    int draws_alg[3]  = {0};

    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            wins_alg[a]   += total_xw[a][b];
            losses_alg[a] += total_ow[a][b];
            draws_alg[a]  += total_dr[a][b];
            wins_alg[b]   += total_ow[a][b];
            losses_alg[b] += total_xw[a][b];
            draws_alg[b]  += total_dr[a][b];
        }
    }

    fprintf(out, "\nresumo por algoritmo (como x e como o somados):\n");
    fprintf(out, "%-12s | %6s %6s %6s | %8s\n", "algoritmo", "wins", "losses", "draws", "winrate");
    fprintf(out, "-------------+----------------------+---------\n");
    for (int a = 0; a < 3; a++) {
        int tot = wins_alg[a] + losses_alg[a] + draws_alg[a];
        fprintf(out, "%-12s | %6d %6d %6d | %7.2f%%\n",
                ALG_NAMES[algs[a]],
                wins_alg[a], losses_alg[a], draws_alg[a],
                100.0 * wins_alg[a] / tot);
    }

    // aqui é o csv da parte 2 só pra usar no relatorio também :)
    fprintf(out, "\ncsv (parte 2 — por par):\n");
    fprintf(out, "algx,algo,x_win,o_win,draw,total\n");
    for (int a = 0; a < 3; a++)
        for (int b = 0; b < 3; b++)
            fprintf(out, "%s,%s,%d,%d,%d,%d\n",
                    ALG_NAMES[algs[a]], ALG_NAMES[algs[b]],
                    total_xw[a][b], total_ow[a][b], total_dr[a][b], total_games);

    fprintf(out, "\ncsv (parte 2 — totais por algoritmo):\n");
    fprintf(out, "algoritmo,wins,losses,draws\n");
    for (int a = 0; a < 3; a++)
        fprintf(out, "%s,%d,%d,%d\n",
                ALG_NAMES[algs[a]], wins_alg[a], losses_alg[a], draws_alg[a]);

    return 0;
}
