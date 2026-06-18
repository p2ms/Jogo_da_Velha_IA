/* 
   contador global de nós explorados.
   usamos essa variável nos três algoritmos pra contar quantos nós cada um visita por chamada. assim dá pra comparar de forma independente do hardware, sem depender só do tempo de execução. adicionamos isso no trabalho pra enriquecer a análise. */

#ifndef NODES_H
#define NODES_H

// variável global compartilhada entre minimax.c, alphabeta.c e astar.c 
extern long long g_nodes;

// zera o contador antes de cada medição 
static inline void reset_nodes(void) { g_nodes = 0; }

// retorna o valor atual do contador 
static inline long long get_nodes(void) { return g_nodes; }

#endif
