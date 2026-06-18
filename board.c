/* 
   implementação das funções do tabuleiro do jogo da velha.
   aqui ficam as funções de verificação, utilidade e impressão.
   esse arquivo foi fornecido pelo senhor (professor) e a gente reutilizou aqui. */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "board.h"

/* verifica se ainda tem pelo menos uma célula vazia no tabuleiro.
   percorre todas as 9 posições procurando um '_'. */
bool isMovesLeft(char b[3][3]) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (b[i][j] == '_')
                return true;
    return false;
}

/* calcula o valor do estado atual do tabuleiro (função utilidade).
   verifica linhas, colunas e diagonais pra saber se alguém venceu.
   retorna +10 se x ganhou, -10 se o ganhou, e 0 se ninguém ganhou ainda. */
int utility(char b[3][3]) {

    // checa as três linhas horizontais
    for (int row = 0; row < 3; row++) {
        if (b[row][0] == b[row][1] && b[row][1] == b[row][2]) {
            if (b[row][0] == 'x') return +10;
            if (b[row][0] == 'o') return -10;
        }
    }

    // checa as três colunas verticais 
    for (int col = 0; col < 3; col++) {
        if (b[0][col] == b[1][col] && b[1][col] == b[2][col]) {
            if (b[0][col] == 'x') return +10;
            if (b[0][col] == 'o') return -10;
        }
    }

    // checa a diagonal principal (canto superior esquerdo ao inferior direito) 
    if (b[0][0] == b[1][1] && b[1][1] == b[2][2]) {
        if (b[0][0] == 'x') return +10;
        if (b[0][0] == 'o') return -10;
    }

    // checa a diagonal secundária (canto superior direito ao inferior esquerdo) 
    if (b[0][2] == b[1][1] && b[1][1] == b[2][0]) {
        if (b[0][2] == 'x') return +10;
        if (b[0][2] == 'o') return -10;
    }

    // ninguém venceu ainda 
    return 0;
}

// realiza uma jogada: coloca o marcador do jogador p na posição m 
void play(char b[3][3], move m, char p) {
    b[m.row][m.col] = p;
}

// imprime o tabuleiro no terminal com uma formatação simples de grade 
void printBoard(char b[3][3]) {
    for (int i = 0; i < 3; i++) {
        printf("|");
        for (int j = 0; j < 3; j++) {
            printf(" %c |", b[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
