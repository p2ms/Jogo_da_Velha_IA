#include <stdbool.h>
#include "board.h"

#ifndef ASTAR_H
#define ASTAR_H

/* valor usado quando todas as linhas do jogador estão bloqueadas pelo adversário,
   ou seja, não tem caminho de vitória disponível. usamos 1000 como "infinito". */
#define ASTAR_INF 1000

/* heurística admissível: retorna o mínimo de marcações que o jogador 'me' ainda precisa colocar pra fechar alguma linha vencedora. só considera linhas que não estão bloqueadas pelo adversário. se todas as linhas estiverem bloqueadas, retorna ASTAR_INF. */
int astarHeuristic(char board[3][3], char me);

/* encontra o melhor movimento usando o a*. faz uma busca com fila de prioridade por f = g + h, onde g é o número de marcações feitas e h é a heurística acima. retorna o primeiro movimento do caminho ótimo encontrado. */
move findBestMoveAStar(char board[3][3], bool isMax);

#endif
