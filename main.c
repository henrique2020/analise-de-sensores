#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "cJson.h"
#include "leitor.h"
#include "processador.h"
#include "visualizacao.h"
#include "fila.h"
#include "logger.h"

#define NUM_ARQUIVOS 2
#define NUM_CIDADES  10

// ==== helpers ====

void exibe_linha_json(cJSON *json, int linha){
    cJSON *primeiro_item = cJSON_GetArrayItem(json, linha);
    if (primeiro_item != NULL) {
        char *string_visualizacao = cJSON_Print(primeiro_item);
        if (string_visualizacao) {
            printf("\n--- Debug: Linha %d ---\n%s\n\n", linha+1, string_visualizacao);
            free(string_visualizacao);
        }
    }
}

void formatar_data(const char *iso, char *saida) {
    if(!iso) return;
    snprintf(saida, 11, "%.2s/%.2s/%.4s", iso + 8, iso + 5, iso);
}

// ==== threads: tipos ====

typedef struct {
    cJSON   *root;
    ARQUIVO *arq;
} ItemProc;

typedef struct {
    Fila    *fila_proc;
    ARQUIVO *arquivos;
    int      n_arquivos;
} CtxLeitora;

typedef struct {
    Fila         *fila_proc;
    ESTATISTICAS *cidades;
    int          *num_cidades;
} CtxStats;

// ==== threads: implementacoes ====

static void *thread_leitora(void *arg) {
    CtxLeitora *ctx = (CtxLeitora*)arg;
    log_msg("LEITORA", "iniciada");

    for (int i = 0; i < ctx->n_arquivos; i++) {
        ARQUIVO *arq = &ctx->arquivos[i];

        log_msg("LEITORA", "carregando arquivo: %s", arq->arquivo);
        cJSON *root = carregar_json(arq->arquivo);
        if (!root) {
            log_msg("LEITORA", "ERRO ao carregar %s", arq->arquivo);
            continue;
        }

        atualizar_payload(root, arq->local_payload);
        log_msg("LEITORA", "%s parseado", arq->arquivo);

        dedup_arquivo(root, arq);
        log_msg("LEITORA", "dedup concluido em %s: %d duplicatas removidas",
                arq->arquivo, arq->duplicatas);

        ItemProc *item = malloc(sizeof(ItemProc));
        item->root = root;
        item->arq  = arq;
        fila_push(ctx->fila_proc, item);
        log_msg("LEITORA", "%s entregue para estatisticas", arq->arquivo);
    }

    fila_close(ctx->fila_proc);
    log_msg("LEITORA", "encerrada");
    return NULL;
}

static void *thread_stats(void *arg) {
    CtxStats *ctx = (CtxStats*)arg;
    log_msg("STATS", "iniciada");

    ItemProc *item;
    while ((item = (ItemProc*)fila_pop(ctx->fila_proc)) != NULL) {
        log_msg("STATS", "processando %s", item->arq->arquivo);

        processar_cidade(item->root, ctx->cidades, ctx->num_cidades, item->arq);

        log_msg("STATS", "%s concluido: %d registros processados",
                item->arq->arquivo, item->arq->registros);

        cJSON_Delete(item->root);
        free(item);
    }

    log_msg("STATS", "encerrada");
    return NULL;
}

// ==== main ====

int main() {
    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    if (log_init("processamento.log") != 0) {
        fprintf(stderr, "Falha ao inicializar log\n");
        return 1;
    }
    log_msg("MAIN", "programa iniciado");

    ESTATISTICAS cidades[NUM_CIDADES] = {0};
    int num_cidades_registradas = 0;
    ARQUIVO arquivos[NUM_ARQUIVOS] = {
        {"mqtt_senzemo_cx_bg.json", "payload",    "created_at",   "id",         "", "", 0, 0},
        {"senzemo_cx_bg.json",      "brute_data", "payload_date", "payload_id", "", "", 0, 0}
    };

    Fila *fila_proc = fila_criar();
    CtxLeitora ctx_l = { fila_proc, arquivos, NUM_ARQUIVOS };
    CtxStats   ctx_s = { fila_proc, cidades, &num_cidades_registradas };

    pthread_t t_leitora, t_stats;
    pthread_create(&t_leitora, NULL, thread_leitora, &ctx_l);
    pthread_create(&t_stats,   NULL, thread_stats,   &ctx_s);
    log_msg("MAIN", "threads de trabalho criadas");

    pthread_join(t_leitora, NULL);
    pthread_join(t_stats,   NULL);
    log_msg("MAIN", "threads de trabalho encerradas");

    fila_destruir(fila_proc);

    exibir_titulo("ANÁLISE DE DADOS DOS SENSORES - CityLivingLab\nProcessamento utilizando pthreads", 1);
    printf("\n");

    for (int i = 0; i < NUM_ARQUIVOS; i++) {
        char min_formatada[11], max_formatada[11];
        formatar_data(arquivos[i].min_dt, min_formatada);
        formatar_data(arquivos[i].max_dt, max_formatada);

        printf("Arquivo analisado:              %s\n", arquivos[i].arquivo);
        printf("Total de registros processados: %d\n", arquivos[i].registros);
        printf("Duplicatas descartadas:         %d\n", arquivos[i].duplicatas);
        printf("Período analisado:              %s a %s\n\n", min_formatada, max_formatada);
    }
    printf("\n");

    exibir_tabelas(cidades, num_cidades_registradas);

    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo_execucao = (fim.tv_sec - inicio.tv_sec)
                          + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    log_msg("MAIN", "tempo total: %.3f segundos", tempo_execucao);

    exibir_titulo("DESEMPENHO", 0);
    printf("Tempo total de execução: %.3f segundos\n", tempo_execucao);
    printf("Threads utilizadas: 3\n");
    printf(" - Thread 1: leitura dos dados\n");
    printf(" - Thread 2: cálculo das estatísticas\n");
    printf(" - Thread 3: registro de logs\n\n");
    printf("Arquivo de log gerado: processamento.log\n");
    printf("\n");

    exibir_titulo("Processamento finalizado com sucesso.", 1);

    log_msg("MAIN", "programa finalizado");
    log_shutdown();
    return 0;
}
