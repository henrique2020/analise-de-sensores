#include <stdio.h>
#include <stdlib.h>
#include "leitor.h"
#include "cJson.h"

cJSON* carregar_json(const char *caminho_arquivo) {
    FILE *file = fopen(caminho_arquivo, "r");
    if (!file) {
        fprintf(stderr, "Erro ao carregar o arquivo %s\n", caminho_arquivo);
        return NULL;
    }

    // Move o ponteiro para o fim para descobrir o tamanho
    fseek(file, 0, SEEK_END);
    long tamanho = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(tamanho + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, tamanho, file);
    fclose(file);
    buffer[tamanho] = '\0';

    cJSON *json = cJSON_Parse(buffer);
    free(buffer); // Liberamos o buffer bruto, pois o cJSON cria sua pr�pria estrutura

    if (!json) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Erro no JSON antes de: %s\n", error_ptr);
        }
    }

    return json;
}

void atualizar_payload(cJSON *root, const char *param) {
    if (!root) return;

    cJSON *it = root->child;
    while (it) {
        cJSON *p_str = cJSON_GetObjectItem(it, param);

        // Se o payload for string ele e convertido
        if (cJSON_IsString(p_str) && p_str->valuestring != NULL) {
            cJSON *p_obj = cJSON_Parse(p_str->valuestring);
            if (p_obj) {
                cJSON_ReplaceItemInObject(it, param, p_obj);
            }
        }
        it = it->next;
    }
}
