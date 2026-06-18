/* 
   definição do contador global de nós explorados.
   o extern em nodes.h deixa os outros arquivos enxergarem essa variável, mas a definição real (que reserva memória) fica aqui. */

#include "nodes.h"

long long g_nodes = 0;
