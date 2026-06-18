/* 
   implementação do algoritmo minimax para o jogo da velha.
   o algoritmo faz uma busca completa em profundidade na árvore de jogo, assumindo que os dois jogadores sempre fazem a melhor jogada possível.
   esse arquivo foi fornecido pelo senhor (professor) e a gente reutilizou aqui. adicionamos só o contador de nós (g_nodes++) pra medir quantos estados o algoritmo visita durante a busca. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "minimax.h"
#include "nodes.h"

// funções auxiliares pra comparar dois inteiros
int max(int a, int b) {
    if (a >= b) return a;
    else return b;
}

int min(int a, int b) {
    if (a <= b) return a;
    else return b;
}

/* algoritmo minimax recursivo.
   a cada chamada, o algoritmo verifica se o jogo acabou (alguém ganhou ou empatou).
   se não acabou, testa todos os movimentos possíveis e escolhe o melhor valor:
   - se for a vez do max (x): quer o maior valor possível
   - se for a vez do min (o): quer o menor valor possível
   depth indica a profundidade atual na árvore (não altera o valor, mas fica disponível
   pra possíveis extensões do algoritmo, como desempate por profundidade). */
int minimax(char board[3][3], int depth, bool isMax) {
    g_nodes++; // conta mais um nó visitado na árvore 

    // verifica se o jogo terminou 
    int score = utility(board);
    if (score == 10)  return score; // x ganhou 
    if (score == -10) return score; // o ganhou 
    if (!isMovesLeft(board)) return 0; // empate 

    // vez do jogador max (x): quer maximizar o valor 
    if (isMax) {
        int best = -1000; // começa com o pior valor possível pra o max 

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == '_') {
                    board[i][j] = 'x'; // testa jogar em (i, j) 
                    best = max(best, minimax(board, depth + 1, false));
                    board[i][j] = '_'; // desfaz o movimento 
                }
            }
        }
        return best;
    }

    // vez do jogador min (o): quer minimizar o valor 
    else {
        int best = 1000; // começa com o pior valor possível pra o min 

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == '_') {
                    board[i][j] = 'o'; // testa jogar em (i, j) 
                    best = min(best, minimax(board, depth + 1, true));
                    board[i][j] = '_'; // desfaz o movimento 
                }
            }
        }
        return best;
    }
}

/* encontra o melhor movimento possível pra o jogador indicado por isMax.
   testa cada célula vazia, simula o movimento, avalia com o minimax e guarda o movimento que levou ao melhor resultado. */
move findBestMove(char board[3][3], bool isMax) {
    move bestMove;
    bestMove.row = -1;
    bestMove.col = -1;

    // valor inicial: o pior possível pra quem vai jogar
    int bestVal = isMax ? -1000 : 1000;

    if (isMax) {
        // jogador x testa cada posição vazia e avalia com o minimax 
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == '_') {
                    board[i][j] = 'x';
                    int moveVal = minimax(board, 0, false);
                    board[i][j] = '_';

                    // atualiza o melhor movimento se encontrou valor maior 
                    if (moveVal > bestVal) {
                        bestMove.row = i;
                        bestMove.col = j;
                        bestVal = moveVal;
                    }
                }
            }
        }
    } else {
        // jogador o testa cada posição vazia e avalia com o minimax 
        int bestVal = 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == '_') {
                    board[i][j] = 'o';
                    int moveVal = minimax(board, 0, true);
                    board[i][j] = '_';

                    // atualiza o melhor movimento se encontrou valor menor 
                    if (moveVal < bestVal) {
                        bestMove.row = i;
                        bestMove.col = j;
                        bestVal = moveVal;
                    }
                }
            }
        }
    }

    printf("valor do melhor movimento : %d\n\n", bestVal);
    return bestMove;
}
