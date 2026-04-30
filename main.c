#include <time.h>
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

void formatar_data(const char *iso, char *saida) {
    if(!iso) return;
    snprintf(saida, 11, "%.2s/%.2s/%.4s", iso + 8, iso + 5, iso);
}

int main() {
    clock_t inicio = clock();

    system("chcp 65001 > nul");

    ESTATISTICAS cidades[NUM_ARQUIVOS] = {0};
    ARQUIVO arquivos[NUM_ARQUIVOS] = {
        {"mqtt_senzemo_cx_bg.json", "payload", "created_at", "", "", 0},
        {"senzemo_cx_bg.json", "brute_data", "payload_date", "", "", 0}
    };

    cJSON *json;
    for (int i = 0; i < NUM_ARQUIVOS; i++) {
        json = carregar_json(arquivos[i].arquivo);
        if (json) {
            atualizar_payload(json, arquivos[i].local_payload);
            //exibe_linha_json(json, 0); // Exibe a primeira linha do JSON para debug
            processar_cidade(json, &cidades[i], &arquivos[i]);
            cJSON_Delete(json);
        }
    }

    exibir_titulo("ANÁLISE DE DADOS DOS SENSORES - CityLivingLab\nProcessamento utilizando pthreads", 1);

    printf("\n\n");

    for (int i = 0; i < NUM_ARQUIVOS; i++) {
        char min_formatada[11], max_formatada[11];
        formatar_data(arquivos[i].min_dt, min_formatada);
        formatar_data(arquivos[i].max_dt, max_formatada);

        printf("Arquivo analisado:              %s\n", arquivos[i].arquivo);
        printf("Total de registros processados: %d\n", arquivos[i].registros);
        printf("Período analisado:              %s a %s\n\n", min_formatada, max_formatada);
    }

    printf("\n");

    exibir_tabelas(cidades, NUM_ARQUIVOS);

    clock_t fim = clock();
    double tempo_execucao = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    exibir_titulo("DESEMPENHO", 0);
    printf("Tempo total de execução: %.3f segundos\n", tempo_execucao);

    printf("\n\n");

    exibir_titulo("Processamento finalizado com sucesso.", 1);

    return 0;
}
