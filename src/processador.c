#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "processador.h"
#include "cJson.h"

// --- Tabela hash para deduplicacao por id unico ---
#define DEDUP_BUCKETS 65536

typedef struct NodoDedup {
    long id;
    struct NodoDedup *prox;
} NodoDedup;

static int dedup_check_insert(NodoDedup **buckets, long id) {
    unsigned k = (unsigned)((unsigned long)id * 2654435761ul) & (DEDUP_BUCKETS - 1);
    NodoDedup *n = buckets[k];
    while (n) {
        if (n->id == id) return 1;
        n = n->prox;
    }
    NodoDedup *novo = malloc(sizeof(NodoDedup));
    if (!novo) return 0;
    novo->id = id;
    novo->prox = buckets[k];
    buckets[k] = novo;
    return 0;
}

static void dedup_free(NodoDedup **buckets) {
    for (int i = 0; i < DEDUP_BUCKETS; i++) {
        NodoDedup *n = buckets[i];
        while (n) {
            NodoDedup *p = n;
            n = n->prox;
            free(p);
        }
        buckets[i] = NULL;
    }
}
// --- fim da tabela hash ---

void adicionar_sf_unico(ESTATISTICAS *est, int sf) {
    for (int i = 0; i < est->qtd_sf; i++) {
        if (est->spreading_factors[i] == sf) {
            return;
        }
    }
    if (est->qtd_sf < 6) {
        est->spreading_factors[est->qtd_sf] = sf;
        est->qtd_sf++;
    }
}

void ordenar_sfs(int *sfs, int qtd) {
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

// Percorre a arvore cJSON e remove fisicamente os nos duplicados pelo id unico.
void dedup_arquivo(cJSON *root, ARQUIVO *arq) {
    if (!root) return;

    NodoDedup **dedup = calloc(DEDUP_BUCKETS, sizeof(NodoDedup*));
    if (!dedup) return;

    cJSON *it = root->child;
    while (it) {
        cJSON *prox = it->next;

        cJSON *id_item = cJSON_GetObjectItem(it, arq->local_id);
        if (id_item && cJSON_IsNumber(id_item)) {
            long id = (long)id_item->valuedouble;
            if (dedup_check_insert(dedup, id)) {
                cJSON_DetachItemViaPointer(root, it);
                cJSON_Delete(it);
                arq->duplicatas++;
            }
        }
        it = prox;
    }

    dedup_free(dedup);
    free(dedup);
}

void processar_cidade(cJSON *root, ESTATISTICAS cidades[], int *num_cidades, ARQUIVO *arq) {
    if (!root) return;

    cJSON *it = cJSON_IsArray(root) ? root->child : root->child;

    while (it) {
        cJSON *payload = cJSON_GetObjectItem(it, arq->local_payload);
        if (!payload) { it = it->next; continue; }

        cJSON *n = cJSON_GetObjectItem(payload, "device_name");
        if (!n || !cJSON_IsString(n)) { it = it->next; continue; }
        const char *nome_cidade = n->valuestring;

        int idx = encontrar_cidade(cidades, *num_cidades, nome_cidade);
        if (idx == -1) {
            if (*num_cidades >= 100) { it = it->next; continue; }
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
                continue;
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
                if (est->bateria_inicial.tempo[0] == '\0' || strcmp(time, est->bateria_inicial.tempo) < 0) {
                    est->bateria_inicial.valor = val;
                    strcpy(est->bateria_inicial.tempo, time);
                }
                if (est->bateria_final.tempo[0] == '\0' || strcmp(time, est->bateria_final.tempo) > 0) {
                    est->bateria_final.valor = val;
                    strcpy(est->bateria_final.tempo, time);
                }
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
