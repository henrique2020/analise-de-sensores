#include "cJson.c"
#include "leitor.c"
#include "processador.c"

#define NUM_ARQUIVOS 2

void exibe_linha_json(cJSON *json, int linha){
    cJSON *primeiro_item = cJSON_GetArrayItem(json, linha);
    if (primeiro_item != NULL) {
        char *string_visualizacao = cJSON_Print(primeiro_item); // Gera a string
        if (string_visualizacao) {
            printf("\n--- Debug: Linha %d ---\n%s\n\n", linha+1, string_visualizacao);
            free(string_visualizacao); // Libera a memória alocada pelo cJSON_Print
        }
    }
}

int main() {
    system("chcp 65001 > nul");

    Estatisticas cidades[NUM_ARQUIVOS] = {0};

    char *arquivos[NUM_ARQUIVOS] = {
        "data/mqtt_senzemo_cx_bg.json",
        "data/senzemo_cx_bg.json"
    };

    char *params[NUM_ARQUIVOS] = {
        "payload",
        "brute_data"
    };

    cJSON *json;
    for (int i = 0; i < NUM_ARQUIVOS; i++) {
        json = carregar_json(arquivos[i]);
        if (json) {
            atualizar_payload(json, params[i]);
            //exibe_linha_json(json, 0); // Exibe a primeira linha do JSON para debug
            processar_cidade(json, &cidades[i], params[i]);
            cJSON_Delete(json);
        }
    }

    exibir_tabelas(cidades, NUM_ARQUIVOS);

    return 0;
}
