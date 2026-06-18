#include <stdbool.h>
#include "board.h"

#ifndef ALPHABETA_H
#define ALPHABETA_H

int alphabeta(char board[3][3], int depth, int alpha, int beta, bool isMax);

/* encontra o melhor movimento usando a poda alfa-beta.
   funciona igual ao findBestMove do minimax, mas chama alphabeta() no lugar. */
move findBestMoveAB(char board[3][3], bool isMax);

#endif
