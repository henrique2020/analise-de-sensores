#ifndef PROCESSADOR_H
#define PROCESSADOR_H

#include "cJson.h"

typedef struct {
    double valor;
    char tempo[30];
} REGISTRO;

typedef struct {
    char nome_cidade[50];
    REGISTRO max_temp, min_temp;
    double soma_temp;
    int cont_temp;

    REGISTRO max_umid, min_umid;
    double soma_umid;
    int cont_umid;

    REGISTRO max_pres, min_pres;
    double soma_pres;
    int cont_pres;

    REGISTRO max_batt, min_batt;

    int spreading_factors[6];
    int qtd_sf;

    int inicializado;
} ESTATISTICAS;

typedef struct {
    char arquivo[100];
    char local_payload[100];
    char local_data[100];
    char max_dt[50], min_dt[50];
    int registros;
} ARQUIVO;

void processar_cidade(cJSON *root, ESTATISTICAS cidades[], int *num_cidades, ARQUIVO *arq);
void exibir_tracejado(int tipo, int tamanho);
void exibir_tracejado_padrao(int tipo);
void exibir_titulo(const char *titulo, int tipo);
void exibir_tabelas(ESTATISTICAS *cidades, int qtd);

#endif
