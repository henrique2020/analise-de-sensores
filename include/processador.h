#ifndef PROCESSADOR_H
#define PROCESSADOR_H

#include "cJson.h"

typedef struct {
    double valor;
    char tempo[30];
} Registro;

typedef struct {
    char nome_cidade[50];
    Registro max_temp, min_temp;
    double soma_temp;
    int cont_temp;

    Registro max_umid, min_umid;
    double soma_umid;
    int cont_umid;

    Registro max_pres, min_pres;
    double soma_pres;
    int cont_pres;

    Registro max_batt, min_batt;

    int spreading_factors[6];
    int qtd_sf;
} Estatisticas;

void processar_cidade(cJSON *root, Estatisticas *est, const char *param);
void exibir_titulo(const char *titulo);
void exibir_tabelas(Estatisticas *cidades, int qtd);

#endif
