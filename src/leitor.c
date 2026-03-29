#include <stdio.h>
#include <stdlib.h>
#include "leitor.h"
#include "cJson.h"

cJSON* carregar_json(const char *caminho_arquivo) {
    FILE *file = fopen(caminho_arquivo, "r");
    if (!file) {
        fprintf(stderr, "Erro: Não foi possível abrir %s\n", caminho_arquivo);
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
    free(buffer); // Liberamos o buffer bruto, pois o cJSON cria sua própria estrutura

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

    cJSON *item = NULL;
    // O seu JSON pode ser um Array direto ou um Objeto com filhos
    cJSON *it = cJSON_IsArray(root) ? root->child : root->child;

    while (it) {
        cJSON *p_str = cJSON_GetObjectItem(it, param);

        // Se o payload for string, a gente converte
        if (cJSON_IsString(p_str) && p_str->valuestring != NULL) {
            cJSON *p_obj = cJSON_Parse(p_str->valuestring);
            if (p_obj) {
                // Substitui a string antiga pelo novo objeto JSON na árvore
                cJSON_ReplaceItemInObject(it, param, p_obj);
            }
        }
        it = it->next;
    }
}
