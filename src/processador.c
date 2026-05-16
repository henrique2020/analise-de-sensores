#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "processador.h"
#include "cJson.h"
#include "logger.h"

// --- Tabela hash para deduplicacao por valores + janela de tempo ---
#define DEDUP_BUCKETS  65536
#define JANELA_DUP_SEG 600    // 10 minutos = registros com mesmos valores
                              // nesse intervalo sao considerados duplicatas

// Chave: cidade + valores das medicoes (em centesimos, para evitar problemas
// de comparacao de double)
typedef struct {
    char cidade[50];
    int  t, h, p, b;    // temperatura, umidade, pressao, bateria * 100
} ChaveDup;

typedef struct NodoDedup {
    ChaveDup chave;
    time_t   timestamp;
    struct NodoDedup *prox;
} NodoDedup;

static unsigned hash_chave(const ChaveDup *k) {
    unsigned h = 17;
    for (int i = 0; k->cidade[i] && i < 12; i++)
        h = h * 31 + (unsigned char)k->cidade[i];
    h = h * 31 + (unsigned)k->t;
    h = h * 31 + (unsigned)k->h;
    h = h * 31 + (unsigned)k->p;
    h = h * 31 + (unsigned)k->b;
    return h & (DEDUP_BUCKETS - 1);
}

static int chave_igual(const ChaveDup *a, const ChaveDup *b) {
    return a->t == b->t && a->h == b->h && a->p == b->p && a->b == b->b
        && strcmp(a->cidade, b->cidade) == 0;
}

// Converte ISO 8601 para time_t (segundos Unix).
static time_t parse_iso_timestamp(const char *iso) {
    if (!iso) return 0;
    struct tm tm = {0};
    if (sscanf(iso, "%d-%d-%dT%d:%d:%d",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) return 0;
    tm.tm_year -= 1900;
    tm.tm_mon  -= 1;
    return timegm(&tm);
}

// Retorna 1 se ja existe registro com a mesma chave em janela < JANELA_DUP_SEG.
// Caso contrario, insere e retorna 0.
static int dedup_check_insert(NodoDedup **buckets, const ChaveDup *chave, time_t ts) {
    unsigned k = hash_chave(chave);
    NodoDedup *n = buckets[k];
    while (n) {
        if (chave_igual(&n->chave, chave)) {
            long diff = (long)(ts - n->timestamp);
            if (diff < 0) diff = -diff;
            if (diff < JANELA_DUP_SEG) {
                return 1;  // duplicata
            }
        }
        n = n->prox;
    }
    NodoDedup *novo = malloc(sizeof(NodoDedup));
    if (!novo) return 0;
    novo->chave = *chave;
    novo->timestamp = ts;
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

// Percorre a arvore cJSON e remove fisicamente os nos que sao duplicatas
// segundo o criterio: mesma cidade, mesmos valores de temperatura, umidade,
// pressao e bateria, e diferenca de tempo < JANELA_DUP_SEG.
void dedup_arquivo(cJSON *root, ARQUIVO *arq) {
    if (!root) return;

    log_msg("LEITORA", "iniciando remocao de duplicatas em %s (janela=%ds)...",
            arq->arquivo, JANELA_DUP_SEG);

    NodoDedup **dedup = calloc(DEDUP_BUCKETS, sizeof(NodoDedup*));
    if (!dedup) return;

    cJSON *it = root->child;
    while (it) {
        cJSON *prox = it->next;

        cJSON *payload = cJSON_GetObjectItem(it, arq->local_payload);
        if (!payload) { it = prox; continue; }

        cJSON *nome = cJSON_GetObjectItem(payload, "device_name");
        cJSON *data_envio = cJSON_GetObjectItem(it, arq->local_data);
        if (!nome || !cJSON_IsString(nome) ||
            !data_envio || !cJSON_IsString(data_envio)) {
            it = prox; continue;
        }

        ChaveDup chave = {0};
        strncpy(chave.cidade, nome->valuestring, sizeof(chave.cidade) - 1);

        // Varre o array "data" extraindo cada medicao em centesimos
        cJSON *data_array = cJSON_GetObjectItem(payload, "data");
        cJSON *leitura;
        cJSON_ArrayForEach(leitura, data_array) {
            cJSON *v   = cJSON_GetObjectItem(leitura, "variable");
            cJSON *val = cJSON_GetObjectItem(leitura, "value");
            if (!v || !cJSON_IsString(v) || !val || !cJSON_IsNumber(val)) continue;
            int centi = (int)(val->valuedouble * 100.0);
            const char *var = v->valuestring;
            if      (strcmp(var, "temperature")  == 0) chave.t = centi;
            else if (strcmp(var, "humidity")     == 0) chave.h = centi;
            else if (strcmp(var, "airpressure")  == 0) chave.p = centi;
            else if (strcmp(var, "batterylevel") == 0) chave.b = centi;
        }

        time_t ts = parse_iso_timestamp(data_envio->valuestring);

        if (dedup_check_insert(dedup, &chave, ts)) {
            cJSON_DetachItemViaPointer(root, it);
            cJSON_Delete(it);
            arq->duplicatas++;
        }

        it = prox;
    }

    dedup_free(dedup);
    free(dedup);

    log_msg("LEITORA", "dedup concluido em %s: %d duplicatas removidas",
            arq->arquivo, arq->duplicatas);
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

    // Loga as estatisticas calculadas, metrica por metrica e cidade por cidade
    for (int i = 0; i < *num_cidades; i++) {
        ESTATISTICAS *est = &cidades[i];
        const char *cidade = est->nome_cidade;

        log_msg("STATS", "calculando temperatura para %s...", cidade);
        if (est->temperatura.tem) {
            double m = est->temperatura.cont > 0
                       ? est->temperatura.soma / est->temperatura.cont : 0;
            log_msg("STATS", "  -> min=%.2f max=%.2f media=%.2f",
                    est->temperatura.min.valor, est->temperatura.max.valor, m);
        }

        log_msg("STATS", "calculando umidade para %s...", cidade);
        if (est->umidade.tem) {
            double m = est->umidade.cont > 0
                       ? est->umidade.soma / est->umidade.cont : 0;
            log_msg("STATS", "  -> min=%.2f max=%.2f media=%.2f",
                    est->umidade.min.valor, est->umidade.max.valor, m);
        }

        log_msg("STATS", "calculando pressao para %s...", cidade);
        if (est->pressao.tem) {
            double m = est->pressao.cont > 0
                       ? est->pressao.soma / est->pressao.cont : 0;
            log_msg("STATS", "  -> min=%.2f max=%.2f media=%.2f",
                    est->pressao.min.valor, est->pressao.max.valor, m);
        }

        log_msg("STATS", "calculando consumo de bateria para %s...", cidade);
        log_msg("STATS", "  -> inicial=%.2fV final=%.2fV consumo=%.2fV",
                est->bateria_inicial.valor,
                est->bateria_final.valor,
                est->bateria_inicial.valor - est->bateria_final.valor);

        log_msg("STATS", "identificando spreading factors usados em %s...", cidade);
        if (est->qtd_sf > 0) {
            char buf[128] = "  -> ";
            int off = 5;
            for (int j = 0; j < est->qtd_sf; j++) {
                off += snprintf(buf + off, sizeof(buf) - off,
                                "SF%d%s",
                                est->spreading_factors[j],
                                j + 1 < est->qtd_sf ? ", " : "");
            }
            log_msg("STATS", "%s", buf);
        }
    }
}
