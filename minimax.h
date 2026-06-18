/* minimax.h
   cabeçalho do algoritmo minimax.
   declara as funções que calculam o melhor movimento pra cada jogador.
   esse arquivo foi feito em aula e a gente reutilizou aqui. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "board.h"

#ifndef MINIMAX_H
#define MINIMAX_H

/* função recursiva do minimax: retorna o valor do melhor estado possível
   a partir do tabuleiro atual, considerando que ambos jogam de forma ótima.
   isMax = true quando é a vez do jogador x (maximizador),
   isMax = false quando é a vez do jogador o (minimizador). */
int minimax(char board[3][3], int depth, bool isMax);

/* percorre todos os movimentos possíveis e retorna o melhor deles
   pra ser jogado pelo jogador indicado por isMax */
move findBestMove(char board[3][3], bool isMax);

#endif
