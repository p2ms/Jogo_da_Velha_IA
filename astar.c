/*
   implementação do algoritmo A* para o jogo da velha.
   aqui o A* busca o caminho mais curto até a vitória pra um jogador, sem considerar os movimentos do adversário (conforme o enunciado). usamos uma fila de prioridade simples (lista com busca linear) e um conjunto fechado pra não revisitar estados já explorados. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "astar.h"
#include "nodes.h"

/* as 8 linhas vencedoras do jogo da velha:
   3 linhas horizontais, 3 colunas verticais e 2 diagonais.
   cada linha é representada por 3 pares (linha, coluna). */
static const int WIN_LINES[8][3][2] = {
    {{0,0},{0,1},{0,2}}, // linha 0 
    {{1,0},{1,1},{1,2}}, // linha 1 
    {{2,0},{2,1},{2,2}}, // linha 2 
    {{0,0},{1,0},{2,0}}, // coluna 0 
    {{0,1},{1,1},{2,1}}, // coluna 1 
    {{0,2},{1,2},{2,2}}, // coluna 2 
    {{0,0},{1,1},{2,2}}, // diagonal principal 
    {{0,2},{1,1},{2,0}}  // diagonal secundária
};

/* heurística do A*: retorna o mínimo de marcações que o jogador 'me' ainda precisa
   pra fechar alguma linha vencedora. só considera linhas sem marcações do adversário,
   pois linhas com peça do oponente estão bloqueadas e não podem ser vencidas.
   se todas as linhas estiverem bloqueadas, retorna ASTAR_INF. */
int astarHeuristic(char board[3][3], char me) {
    char opp = (me == 'x') ? 'o' : 'x';
    int best = ASTAR_INF;

    for (int l = 0; l < 8; l++) {
        int mine = 0, theirs = 0;

        // conta quantas marcações de cada jogador estão na linha l 
        for (int k = 0; k < 3; k++) {
            int r = WIN_LINES[l][k][0];
            int c = WIN_LINES[l][k][1];
            if (board[r][c] == me)  mine++;
            else if (board[r][c] == opp) theirs++;
        }

        // só considera linhas não bloqueadas pelo adversário */
        if (theirs == 0) {
            int needed = 3 - mine; // quantas marcações faltam pra fechar essa linha */
            if (needed < best) best = needed;
        }
    }
    return best;
}

/* converte o tabuleiro em um número inteiro entre 0 e 3^9 - 1 (19682).
   serve como chave única pra identificar um estado no conjunto fechado.
   cada célula vira um dígito na base 3: '_' = 0, 'x' = 1, 'o' = 2. */
static int boardKey(char b[3][3]) {
    int k = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int v = (b[i][j] == '_') ? 0 : (b[i][j] == 'x') ? 1 : 2;
            k = k * 3 + v;
        }
    }
    return k;
}

// número máximo de chaves possíveis: 3^9 = 19683 estados distintos 
#define ASTAR_KEYS    19683
//tamanho máximo da lista aberta (fronteira de busca) 
#define ASTAR_OPEN_MAX 50000

/* estrutura de um nó da busca A*.
   guarda o estado do tabuleiro, os valores g (custo acumulado) e h (heurística),
   e qual foi o primeiro movimento feito a partir do estado inicial,
   pra saber o que retornar ao final. */
typedef struct {
    char board[3][3]; // estado do tabuleiro nesse nó 
    int  g;          // quantas marcações o jogador fez até aqui 
    int  h;           // estimativa de quantas marcações ainda faltam pra vencer 
    int  firstRow;   // linha do primeiro movimento feito a partir da raiz 
    int  firstCol;    // coluna do primeiro movimento feito a partir da raiz 
} Node;

/* busca A* da posição atual até o estado em que o jogador 'me' venceu.
   expande apenas os movimentos do próprio jogador, ignorando o adversário.
   usa fila de prioridade por f = g + h (busca linear na lista, que é pequena).
   usa um conjunto fechado (visited) pra não revisitar estados.
   retorna o primeiro movimento do caminho ótimo encontrado.
   se não encontrar caminho de vitória, joga na primeira célula vazia. */
move findBestMoveAStar(char board[3][3], bool isMax) {
    char me = isMax ? 'x' : 'o';
    move fallback = { -1, -1 };

    // se o jogo já terminou não tem o que fazer 
    if (utility(board) != 0 || !isMovesLeft(board)) return fallback;

    // lista aberta (fronteira) e conjunto fechado (estados já expandidos) 
    static Node openList[ASTAR_OPEN_MAX];
    static char visited[ASTAR_KEYS];
    memset(visited, 0, sizeof(visited)); // limpa o conjunto fechado 
    int openSize = 0;

    /* expansão inicial: testa cada célula vazia como primeiro movimento possível.
       guarda qual foi o primeiro movimento em firstRow/firstCol pra recuperar depois. */
    for (int i = 0; i < 3 && openSize < ASTAR_OPEN_MAX; i++) {
        for (int j = 0; j < 3 && openSize < ASTAR_OPEN_MAX; j++) {
            if (board[i][j] == '_') {
                Node n;
                memcpy(n.board, board, 9);
                n.board[i][j] = me;
                n.g = 1;
                n.h = astarHeuristic(n.board, me);
                n.firstRow = i;
                n.firstCol = j;
                // se a heurística é infinita, esse primeiro movimento não leva à vitória 
                if (n.h == ASTAR_INF) continue;
                openList[openSize++] = n;
            }
        }
    }

    // se nenhum movimento inicial leva à vitória, joga na primeira célula vazia 
    if (openSize == 0) {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (board[i][j] == '_') {
                    move m = { i, j };
                    return m;
                }
        return fallback;
    }

    // loop principal do a*: expande o nó com menor f = g + h 
    while (openSize > 0) {
        // encontra o nó de menor f na lista (busca linear) 
        int bestIdx = 0;
        int bestF   = openList[0].g + openList[0].h;
        for (int i = 1; i < openSize; i++) {
            int f = openList[i].g + openList[i].h;
            if (f < bestF) { bestF = f; bestIdx = i; }
        }

        // remove o melhor nó da lista aberta 
        Node cur = openList[bestIdx];
        openList[bestIdx] = openList[--openSize];

        // verifica se o estado já foi expandido antes 
        int key = boardKey(cur.board);
        if (visited[key]) continue;
        visited[key] = 1;
        g_nodes++; // conta mais um nó expandido 

        // chegamos ao objetivo: jogador 'me' tem três em linha 
        if (cur.h == 0) {
            move m = { cur.firstRow, cur.firstCol };
            return m;
        }

        // expansão: adiciona uma marcação de 'me' em cada célula vazia restante 
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (cur.board[i][j] == '_' && openSize < ASTAR_OPEN_MAX) {
                    Node n;
                    memcpy(n.board, cur.board, 9);
                    n.board[i][j] = me;
                    n.g = cur.g + 1;
                    n.h = astarHeuristic(n.board, me);
                    if (n.h == ASTAR_INF) continue; // linha bloqueada, ignora 
                    n.firstRow = cur.firstRow; // mantém o primeiro movimento da raiz 
                    n.firstCol = cur.firstCol;
                    if (visited[boardKey(n.board)]) continue; // já foi expandido 
                    openList[openSize++] = n;
                }
            }
        }
    }

    // sem caminho de vitória encontrado: joga na primeira célula vazia 
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == '_') {
                move m = { i, j };
                return m;
            }
    return fallback;
}
