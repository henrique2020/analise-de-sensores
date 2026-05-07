#ifndef PROCESSADOR_H
#define PROCESSADOR_H

#include "cJson.h"

typedef struct {
    double valor;
    char tempo[30];
} REGISTRO;

typedef struct {
    REGISTRO max;
    REGISTRO min;
    double soma;
    int cont;
    int tem;
} METRICA;

typedef struct {
    char nome_cidade[50];
    METRICA temperatura;
    METRICA umidade;
    METRICA pressao;

    REGISTRO bateria_inicial;
    REGISTRO bateria_final;

    int spreading_factors[6];
    int qtd_sf;

    int inicializado;
} ESTATISTICAS;

typedef struct {
    char arquivo[100];
    char local_payload[100];
    char local_data[100];
    char local_id[100];
    char max_dt[50], min_dt[50];
    int registros;
    int duplicatas;
} ARQUIVO;

void processar_cidade(cJSON *root, ESTATISTICAS cidades[], int *num_cidades, ARQUIVO *arq);
void dedup_arquivo(cJSON *root, ARQUIVO *arq);
void ordenar_sfs(int *sfs, int qtd);

#endif
