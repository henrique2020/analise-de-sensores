#include <stdlib.h>
#include <pthread.h>
#include "fila.h"

typedef struct No {
    void *item;
    struct No *prox;
} No;

struct Fila {
    No *head;
    No *tail;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int fechada;
};

Fila *fila_criar(void) {
    Fila *f = malloc(sizeof(Fila));
    if (!f) return NULL;
    f->head = NULL;
    f->tail = NULL;
    f->fechada = 0;
    pthread_mutex_init(&f->mutex, NULL);
    pthread_cond_init(&f->cond, NULL);
    return f;
}

void fila_push(Fila *f, void *item) {
    if (!f) return;
    No *n = malloc(sizeof(No));
    if (!n) return;
    n->item = item;
    n->prox = NULL;

    pthread_mutex_lock(&f->mutex);
    if (f->tail) f->tail->prox = n;
    else         f->head = n;
    f->tail = n;
    pthread_cond_signal(&f->cond);
    pthread_mutex_unlock(&f->mutex);
}

void *fila_pop(Fila *f) {
    if (!f) return NULL;
    pthread_mutex_lock(&f->mutex);

    while (!f->head && !f->fechada) {
        pthread_cond_wait(&f->cond, &f->mutex);
    }

    if (!f->head) {
        pthread_mutex_unlock(&f->mutex);
        return NULL;
    }

    No *n = f->head;
    f->head = n->prox;
    if (!f->head) f->tail = NULL;

    void *item = n->item;
    free(n);

    pthread_mutex_unlock(&f->mutex);
    return item;
}

void fila_close(Fila *f) {
    if (!f) return;
    pthread_mutex_lock(&f->mutex);
    f->fechada = 1;
    pthread_cond_broadcast(&f->cond);
    pthread_mutex_unlock(&f->mutex);
}

void fila_destruir(Fila *f) {
    if (!f) return;
    No *n = f->head;
    while (n) {
        No *p = n;
        n = n->prox;
        free(p);
    }
    pthread_mutex_destroy(&f->mutex);
    pthread_cond_destroy(&f->cond);
    free(f);
}
