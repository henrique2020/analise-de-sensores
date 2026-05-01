#include <stdio.h>
#include <string.h>
#include "processador.h"
#include "cJson.h"

void adicionar_sf_unico(ESTATISTICAS *est, int sf) {
    // Verifica se o SF já foi armazenado
    for (int i = 0; i < est->qtd_sf; i++) {
        if (est->spreading_factors[i] == sf) {
            return; // SF já existe
        }
    }
    // Adiciona o novo SF se houver espaço
    if (est->qtd_sf < 6) {
        est->spreading_factors[est->qtd_sf] = sf;
        est->qtd_sf++;
    }
}

void ordenar_sfs(int *sfs, int qtd) {
    // Ordenação por inserção para arrays pequenos
    for (int i = 1; i < qtd; i++) {
        int chave = sfs[i];
        int j = i - 1;
        while (j >= 0 && sfs[j] > chave) {
            sfs[j + 1] = sfs[j];
            j--;
        }
        sfs[j + 1] = chave;
    }
}

int encontrar_cidade(ESTATISTICAS cidades[], int num_cidades, const char *nome) {
    for (int i = 0; i < num_cidades; i++) {
        if (strcmp(cidades[i].nome_cidade, nome) == 0) {
            return i;
        }
    }
    return -1;
}

void processar_cidade(cJSON *root, ESTATISTICAS cidades[], int *num_cidades, ARQUIVO *arq) {
    if (!root) return;

    // Itera sobre os objetos (seja array ou os filhos do objeto raiz)
    cJSON *it = cJSON_IsArray(root) ? root->child : root->child;

    while (it) {
        cJSON *payload = cJSON_GetObjectItem(it, arq->local_payload);
        if (!payload) { it = it->next; continue; }

        // Nome da cidade
        cJSON *n = cJSON_GetObjectItem(payload, "device_name");
        if (!n || !cJSON_IsString(n)) { it = it->next; continue; }
        const char *nome_cidade = n->valuestring;

        int idx = encontrar_cidade(cidades, *num_cidades, nome_cidade);
        if (idx == -1) {
            if (*num_cidades >= 100) { it = it->next; continue; } // Limite de cidades
            idx = *num_cidades;
            memset(&cidades[idx], 0, sizeof(ESTATISTICAS));
            strcpy(cidades[idx].nome_cidade, nome_cidade);
            (*num_cidades)++;
        }

        ESTATISTICAS *est = &cidades[idx];

        cJSON *data_envio = cJSON_GetObjectItem(it, arq->local_data);
        if (data_envio && cJSON_IsString(data_envio)) {
            const char *data_str = data_envio->valuestring;
            if (arq->max_dt[0] == '\0' || strcmp(arq->max_dt, data_str) < 0) {
                strcpy(arq->max_dt, data_str);
            }
            if (arq->min_dt[0] == '\0' || strcmp(arq->min_dt, data_str) > 0) {
                strcpy(arq->min_dt, data_str);
            }
        }

        cJSON *data_array = cJSON_GetObjectItem(payload, "data");
        cJSON *leitura;

        cJSON_ArrayForEach(leitura, data_array) {
            cJSON *var_item = cJSON_GetObjectItem(leitura, "variable");
            cJSON *val_item = cJSON_GetObjectItem(leitura, "value");
            cJSON *time_item = cJSON_GetObjectItem(leitura, "time");

            if (!var_item || !cJSON_IsString(var_item) ||
                !val_item || !cJSON_IsNumber(val_item) ||
                !time_item || !cJSON_IsString(time_item)) {
                continue; // Pula itens inválidos
            }

            const char *var = var_item->valuestring;
            double val = val_item->valuedouble;
            const char *time = time_item->valuestring;

            if (strcmp(var, "temperature") == 0) {
                if (!est->temperatura.tem || val > est->temperatura.max.valor) {
                    est->temperatura.max.valor = val;
                    strcpy(est->temperatura.max.tempo, time);
                }
                if (!est->temperatura.tem || val < est->temperatura.min.valor) {
                    est->temperatura.min.valor = val;
                    strcpy(est->temperatura.min.tempo, time);
                }
                est->temperatura.soma += val;
                est->temperatura.cont++;
                est->temperatura.tem = 1;
            }
            else if (strcmp(var, "humidity") == 0) {
                if (!est->umidade.tem || val > est->umidade.max.valor) {
                    est->umidade.max.valor = val;
                    strcpy(est->umidade.max.tempo, time);
                }
                if (!est->umidade.tem || val < est->umidade.min.valor) {
                    est->umidade.min.valor = val;
                    strcpy(est->umidade.min.tempo, time);
                }
                est->umidade.soma += val;
                est->umidade.cont++;
                est->umidade.tem = 1;
            }
            else if (strcmp(var, "airpressure") == 0) {
                if (!est->pressao.tem || val > est->pressao.max.valor) {
                    est->pressao.max.valor = val;
                    strcpy(est->pressao.max.tempo, time);
                }
                if (!est->pressao.tem || val < est->pressao.min.valor) {
                    est->pressao.min.valor = val;
                    strcpy(est->pressao.min.tempo, time);
                }
                est->pressao.soma += val;
                est->pressao.cont++;
                est->pressao.tem = 1;
            }
            else if (strcmp(var, "batterylevel") == 0) {
                if (!est->bateria.tem || val > est->bateria.max.valor) { est->bateria.max.valor = val; }
                if (!est->bateria.tem || val < est->bateria.min.valor) { est->bateria.min.valor = val; }
                est->bateria.tem = 1;
            }
            else if (strcmp(var, "lora_spreading_factor") == 0) {
                int sf = (int)val;
                adicionar_sf_unico(est, sf);
            }
        }
        est->inicializado = 1;
        it = it->next;
        arq->registros++;
    }
}
