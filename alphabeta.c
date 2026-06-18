/* 
   implementação da poda alfa-beta para o jogo da velha.
   a estrutura é quase idêntica ao minimax, mas com dois parâmetros extras:
   alpha e beta, que permitem cortar ramos da árvore sem perder o resultado ótimo.
   implementamos isso, reutilizando a lógica do minimax da aula. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "alphabeta.h"
#include "nodes.h"

/* usamos static aqui pra evitar conflito com as funções max/min do minimax.c (os dois arquivos são compilados juntos e não podem ter funções com o mesmo nome) */
static int max_ab(int a, int b) { return (a >= b) ? a : b; }
static int min_ab(int a, int b) { return (a <= b) ? a : b; }

/* algoritmo minimax com poda alfa-beta.
   a ideia aqui é a mesma do minimax: explorar a árvore de jogo recursivamente.
   a diferença é que mantemos alpha e beta pra saber quando um ramo pode ser ignorado:
   - alpha: o maior valor que o max já encontrou em outro lugar
   - beta:  o menor valor que o min já encontrou em outro lugar
   se em qualquer momento beta <= alpha, o ramo atual não vai mudar o resultado,
   então podamos (cortamos) e retornamos logo. */
int alphabeta(char board[3][3], int depth, int alpha, int beta, bool isMax) {
    g_nodes++; /* conta mais um nó visitado */

    // verifica se o jogo terminou
    int score = utility(board);
    if (score == 10)  return score; // x ganhou 
    if (score == -10) return score; // o ganhou 
    if (!isMovesLeft(board)) return 0; // empate 

    // vez do max (x): quer o maior valor possível 
    if (isMax) {
        int best = -1000;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == '_') {
                    board[i][j] = 'x';
                    best = max_ab(best, alphabeta(board, depth + 1, alpha, beta, false));
                    board[i][j] = '_';

                    // atualiza alpha com o melhor valor do max até agora 
                    alpha = max_ab(alpha, best);

                    // poda: o min nunca vai deixar chegar aqui, pode cortar 
                    if (beta <= alpha) return best;
                }
            }
        }
        return best;
    }

    // vez do min (o): quer o menor valor possível
    else {
        int best = 1000;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == '_') {
                    board[i][j] = 'o';
                    best = min_ab(best, alphabeta(board, depth + 1, alpha, beta, true));
                    board[i][j] = '_';

                    // atualiza beta com o menor valor do min até agora
                    beta = min_ab(beta, best);

                    // poda: o max nunca vai deixar chegar aqui, pode cortar
                    if (beta <= alpha) return best;
                }
            }
        }
        return best;
    }
}

/* encontra o melhor movimento usando a poda alfa-beta.
   percorre todas as células vazias, testa cada jogada possível e avalia
   com alphabeta(). no início, alpha e beta começam nos extremos (-1000 e +1000)
   pra que a poda funcione corretamente desde o primeiro nível. */
move findBestMoveAB(char board[3][3], bool isMax) {
    move bestMove;
    bestMove.row = -1;
    bestMove.col = -1;

    char p = isMax ? 'x' : 'o'; // qual marcador vai ser usado 
    int bestVal = isMax ? -1000 : 1000;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == '_') {
                board[i][j] = p;
                // avalia o movimento com alpha e beta nos extremos 
                int moveVal = alphabeta(board, 0, -1000, 1000, !isMax);
                board[i][j] = '_';

                // atualiza o melhor movimento conforme o jogador 
                if (isMax ? (moveVal > bestVal) : (moveVal < bestVal)) {
                    bestMove.row = i;
                    bestMove.col = j;
                    bestVal = moveVal;
                }
            }
        }
    }
    return bestMove;
}
