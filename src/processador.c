#include <stdio.h>
#include <string.h>
#include "processador.h"
#include "cJson.h"

void processar_cidade(cJSON *root, Estatisticas *est, const char *param) {
    if (!root) return;
    int init = 0;

    // Itera sobre os objetos (seja array ou os filhos do objeto raiz)
    cJSON *it = cJSON_IsArray(root) ? root->child : root->child;

    while (it) {
        cJSON *payload = cJSON_GetObjectItem(it, param);
        if (!payload) { it = it->next; continue; }

        // Nome da cidade
        if (est->nome_cidade[0] == '\0') {
            cJSON *n = cJSON_GetObjectItem(payload, "device_name");
            if (n) strcpy(est->nome_cidade, n->valuestring);
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
                if (!init || val > est->max_temp.valor) { est->max_temp.valor = val; strcpy(est->max_temp.tempo, time); }
                if (!init || val < est->min_temp.valor) { est->min_temp.valor = val; strcpy(est->min_temp.tempo, time); }
                est->soma_temp += val; est->cont_temp++;
            }
            else if (strcmp(var, "humidity") == 0) {
                if (!init || val > est->max_umid.valor) { est->max_umid.valor = val; strcpy(est->max_umid.tempo, time); }
                if (!init || val < est->min_umid.valor) { est->min_umid.valor = val; strcpy(est->min_umid.tempo, time); }
                est->soma_umid += val; est->cont_umid++;
            }
            else if (strcmp(var, "airpressure") == 0) {
                if (!init || val > est->max_pres.valor) { est->max_pres.valor = val; strcpy(est->max_pres.tempo, time); }
                if (!init || val < est->min_pres.valor) { est->min_pres.valor = val; strcpy(est->min_pres.tempo, time); }
                est->soma_pres += val; est->cont_pres++;
            }
        }
        init = 1;
        it = it->next;
    }
}

void exibir_tabelas(Estatisticas *cidades, int qtd) {
    const char *metricas[] = {"TEMPERATURA (°C)", "UMIDADE (%)", "PRESSÃO ATMOSFÉRICA (hPa)"};

    for (int m = 0; m < 3; m++) {
        printf("\n------------------------------------------------------------\n");
        printf("%s\n", metricas[m]);
        printf("------------------------------------------------------------\n");
        printf("%-20s | %-7s | %-24s | %-7s | %-24s | %-7s\n",
               "Cidade", "Mínima", "Data/Hora", "Máxima", "Data/Hora", "Média");
        printf("------------------------------------------------------------------------------------------------------\n");

        for (int i = 0; i < qtd; i++) {
            double min, max, media = 0;
            char *dMin, *dMax;

            // Seleciona os dados baseados na métrica atual do loop
            if (m == 0) { // Temperatura
                min = cidades[i].min_temp.valor;
                max = cidades[i].max_temp.valor;
                dMin = cidades[i].min_temp.tempo;
                dMax = cidades[i].max_temp.tempo;
                if (cidades[i].cont_temp > 0)
                    media = cidades[i].soma_temp / cidades[i].cont_temp;
            }
            else if (m == 1) { // Umidade
                min = cidades[i].min_umid.valor;
                max = cidades[i].max_umid.valor;
                dMin = cidades[i].min_umid.tempo;
                dMax = cidades[i].max_umid.tempo;
                if (cidades[i].cont_umid > 0)
                    media = cidades[i].soma_umid / cidades[i].cont_umid;
            }
            else { // Pressão
                min = cidades[i].min_pres.valor;
                max = cidades[i].max_pres.valor;
                dMin = cidades[i].min_pres.tempo;
                dMax = cidades[i].max_pres.tempo;
                if (cidades[i].cont_pres > 0)
                    media = cidades[i].soma_pres / cidades[i].cont_pres;
            }

            // Imprime a linha da cidade formatada
            // %-20s -> Alinha string à esquerda com 20 espaços
            // %-7.2f -> Alinha número com 2 casas decimais em 7 espaços
            printf("%-20s | %-7.2f | %-22s | %-7.2f | %-22s | %-7.2f\n",
                   cidades[i].nome_cidade, min, dMin, max, dMax, media);
        }
    }
    printf("\n");
}
