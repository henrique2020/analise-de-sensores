#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include "fila.h"
#include "logger.h"

static FILE     *log_file   = NULL;
static Fila     *log_fila   = NULL;
static pthread_t log_thread;
static int       log_ativo  = 0;

static void *thread_logger_main(void *arg) {
    (void)arg;
    char *msg;
    while ((msg = (char*)fila_pop(log_fila)) != NULL) {
        fputs(msg, log_file);
        free(msg);
    }
    return NULL;
}

int log_init(const char *caminho) {
    log_file = fopen(caminho, "w");
    if (!log_file) return -1;
    setvbuf(log_file, NULL, _IOLBF, 0);

    log_fila = fila_criar();
    if (!log_fila) {
        fclose(log_file);
        log_file = NULL;
        return -1;
    }

    if (pthread_create(&log_thread, NULL, thread_logger_main, NULL) != 0) {
        fila_destruir(log_fila);
        log_fila = NULL;
        fclose(log_file);
        log_file = NULL;
        return -1;
    }

    log_ativo = 1;
    return 0;
}

void log_msg(const char *origem, const char *formato, ...) {
    if (!log_ativo) return;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    time_t t_local = ts.tv_sec - 3 * 3600;
    struct tm tm_local;
    gmtime_r(&t_local, &tm_local);

    char data_hora[32];
    strftime(data_hora, sizeof(data_hora), "%Y-%m-%d %H:%M:%S", &tm_local);

    char corpo[512];
    va_list args;
    va_start(args, formato);
    vsnprintf(corpo, sizeof(corpo), formato, args);
    va_end(args);

    char *msg = malloc(700);
    if (!msg) return;
    snprintf(msg, 700, "[%s.%03ld] [%-8s] %s\n",
             data_hora, ts.tv_nsec / 1000000L,
             origem ? origem : "?",
             corpo);

    fila_push(log_fila, msg);
}

void log_shutdown(void) {
    if (!log_ativo) return;

    fila_close(log_fila);
    pthread_join(log_thread, NULL);
    fila_destruir(log_fila);
    log_fila = NULL;

    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    log_ativo = 0;
}
