# Análise de Dados de Sensores - CityLivingLab

Software em **C** com **pthreads** para análise de medições (temperatura,
umidade, pressão atmosférica e bateria) coletadas por sensores instalados
em Caxias do Sul e Bento Gonçalves, parte do projeto
[CityLivingLab](https://www.citylivinglab.com).

O programa lê arquivos JSON, deduplica registros, calcula estatísticas
agregadas por cidade e exibe um relatório no terminal, gerando também
um arquivo de log para auditoria.

## Pré-requisitos

- Linux (testado em Ubuntu 22.04+)
- `gcc` (qualquer versão recente com suporte a C99)
- `make`
- Suporte a `pthreads` (presente por padrão em qualquer distro Linux)

## Estrutura do projeto

​```
.
├── Makefile                    # Build do projeto
├── main.c                      # Ponto de entrada e orquestração de threads
├── data/                       # Arquivos JSON de entrada (não versionados)
│   ├── mqtt_senzemo_cx_bg.json
│   └── senzemo_cx_bg.json
├── include/                    # Headers públicos
│   ├── cJson.h                 # Lib cJSON (parser de JSON)
│   ├── fila.h                  # Fila genérica thread-safe
│   ├── leitor.h                # Carga de arquivos JSON
│   ├── logger.h                # API do logger assíncrono
│   ├── processador.h           # Tipos e funções de processamento
│   └── visualizacao.h          # Saída formatada
└── src/                        # Implementações
    ├── cJson.c
    ├── fila.c                  # Fila com mutex + variável de condição
    ├── leitor.c
    ├── logger.c                # Thread de log e gravação no arquivo
    ├── processador.c           # Dedup + cálculo de estatísticas
    └── visualizacao.c
​```
## Como obter os dados

Os arquivos JSON não fazem parte do repositório (são grandes demais).
Baixe pelo link do enunciado e coloque os dois arquivos dentro da pasta `data/`:

- `mqtt_senzemo_cx_bg.json`
- `senzemo_cx_bg.json`

[Link de download dos JSONs](https://drive.google.com/file/d/1BkaovTMQS00TYr_LemH9GqtW7rjGJ5vi/view?usp=sharing)

## Compilação

Da raiz do projeto:

```bash
make
```

Para limpar artefatos de compilação:

```bash
make clean
```

## Execução

```bash
./analise-de-sensores
```

A saída é exibida diretamente no terminal seguindo o template do enunciado.
Um arquivo `processamento.log` é gerado na raiz com eventos do processamento.

## Arquitetura

O programa usa **3 threads** (além da thread principal), conforme exigido:

​```
        ┌──────────────────────┐
        │  Thread LEITORA      │
        │  - carrega JSON      │  arquivo 1 ─┐
        │  - parseia payloads  │             ├─► fila_proc
        │  - dedup por id      │  arquivo 2 ─┘
        └──────────┬───────────┘
                   ▼
        ┌──────────────────────┐
        │  Thread ESTATÍSTICAS │
        │  - min/max/média     │
        │  - bateria, SFs      │
        └──────────────────────┘

        ┌──────────────────────┐
        │  Thread LOGGER       │ ◄── log_msg()
        │  - escreve no arquivo│     (chamado de qualquer thread)
        └──────────────────────┘
​```

    A comunicação entre as threads é feita por uma **fila produtor-consumidor**
(`mutex` + `pthread_cond`). A leitora produz para a fila enquanto as
estatísticas consomem em paralelo, dando *pipelining* real entre os
arquivos. O logger consome de uma fila própria e grava em disco
de forma assíncrona, sem bloquear as outras threads.

### Decisões de design

- **Deduplicação**: por `id` (em `mqtt_*`) e `payload_id` (em `senzemo_*`),
  usando uma tabela hash com 65.536 buckets. Itens duplicados são removidos
  fisicamente da árvore cJSON antes do cálculo de estatísticas.
- **Bateria**: o consumo é calculado pela diferença entre a primeira e a
  última leitura **cronológicas** (não max/min absolutos), pois a tensão
  pode flutuar com temperatura e exposição solar.
- **Fuso horário**: os timestamps internos do JSON estão em UTC. As datas
  exibidas e logadas são convertidas para UTC-3 (Brasília).
- **Medição de tempo**: `clock_gettime(CLOCK_MONOTONIC)` em vez de `clock()`,
  pois com pthreads o `clock()` somaria o tempo de CPU de todas as threads.

## Saída esperada

Tempo total de execução típico: **~0.6 segundos** (Ubuntu, CPU moderna).

O relatório segue o template do enunciado, com seções para temperatura,
umidade, pressão, bateria, spreading factors e desempenho. Veja exemplo
de log em `processamento.log` após a execução.

## Autores

- Fabio Augusto Calgaro
- Henrique Britz Hahn
