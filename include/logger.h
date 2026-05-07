#ifndef LOGGER_H
#define LOGGER_H

// Logger assincrono. As threads chamam log_msg() e a thread interna
// do logger se encarrega de gravar no arquivo, sem bloquear ninguem.

// Inicializa o subsistema: abre o arquivo de log e dispara a thread.
// Retorna 0 em sucesso, -1 em erro.
int log_init(const char *caminho);

// Empilha uma mensagem para o logger gravar.
// Thread-safe. Pode ser chamada de qualquer thread.
//
// Exemplo:
//   log_msg("LEITORA", "carregado %s com %d registros", arquivo, n);
void log_msg(const char *origem, const char *formato, ...);

// Encerra o subsistema: fecha a fila, espera a thread drenar tudo
// e fecha o arquivo de log.
void log_shutdown(void);

#endif
