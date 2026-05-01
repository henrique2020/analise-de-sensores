#ifndef VISUALIZACAO_H
#define VISUALIZACAO_H

#include "processador.h"

void exibir_tracejado(int tipo, int tamanho);
void exibir_tracejado_padrao(int tipo);
void exibir_titulo(const char *titulo, int tipo);
void exibir_tabelas(ESTATISTICAS *cidades, int qtd);

#endif
