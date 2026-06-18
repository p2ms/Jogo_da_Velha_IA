#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef BOARD_H
#define BOARD_H

// representa uma jogada: linha e coluna escolhidas pelo jogador 
typedef struct {
    int row, col;
} move;

// retorna true se ainda tem célula vazia no tabuleiro 
bool isMovesLeft(char board[3][3]);

// calcula a utilidade do estado atual: +10 se x venceu, -10 se o venceu, 0 caso contrário 
int utility(char b[3][3]);

// coloca o marcador do jogador p na posição indicada por m 
void play(char b[3][3], move m, char p);

// imprime o tabuleiro no terminal pra gente visualizar o jogo 
void printBoard(char b[3][3]);

#endif
