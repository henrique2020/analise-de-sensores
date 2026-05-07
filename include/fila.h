#ifndef FILA_H
#define FILA_H

// Fila generica thread-safe (produtor-consumidor).
// Armazena ponteiros opacos (void*) - cabe ao chamador gerenciar a memoria
// dos itens em si.

typedef struct Fila Fila;

// Cria uma nova fila vazia. Retorna NULL em caso de falha.
Fila *fila_criar(void);

// Libera os recursos da fila (mutex, cond, no's restantes).
// Nao libera os itens em si - se sobraram itens, o chamador deve drenar antes.
void fila_destruir(Fila *f);

// Adiciona um item ao final da fila e sinaliza um consumidor adormecido.
void fila_push(Fila *f, void *item);

// Remove e retorna o primeiro item da fila.
// Bloqueia se estiver vazia, ate haver item ou a fila ser fechada.
// Retorna NULL quando a fila esta fechada E vazia (fim do consumo).
void *fila_pop(Fila *f);

// Marca a fila como fechada e acorda todos os consumidores adormecidos.
// Apos fechada, push tem efeito mas pop retorna NULL quando esvaziar.
void fila_close(Fila *f);

#endif
