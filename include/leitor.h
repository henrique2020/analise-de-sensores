#ifndef LEITOR_H
#define LEITOR_H

#include "cJson.h"

// Lê o arquivo bruto e faz o primeiro parse
cJSON* carregar_json(const char *caminho);

// Percorre o JSON e onde encontrar "{param}" como string, converte para objeto
void atualizar_payload(cJSON *root, const char *param);

#endif
