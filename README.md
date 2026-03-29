# Descrição

O objetivo deste trabalho é, em duplas ou trios, desenvolver um pequeno software em C utilizando pthreads capaz de ler arquivos no formato JSON contendo medições de temperatura, umidade, nível de bateria e pressão atmosférica, coletadas por sensores instalados nas cidades de Caxias do Sul e Bento Gonçalves ao longo do último ano.

> Ambos os sensores fazem parte de um projeto conduzido pelo grupo de pesquisa [CityLivingLab](https://www.citylivinglab.com "https://www.citylivinglab.com").

# Requisitos

1. O programa deve utilizar múltiplas `threads (pthreads)` para separar pelo menos as seguintes etapas do processamento: leitura dos dados, cálculo das estatísticas e registro dos logs.
2. Ler os arquivos JSON disponível no [Google Drive](https://drive.google.com/file/d/1BkaovTMQS00TYr_LemH9GqtW7rjGJ5vi/view?usp=sharing "https://drive.google.com/file/d/1BkaovTMQS00TYr_LemH9GqtW7rjGJ5vi/view?usp=sharing")
3. Para cada uma das cidades, identificar e apresentar em tela a maior e a menor temperatura registradas, bem como a(s) data(s) e horário(s) em que essas medições ocorreram.
4. Para cada uma das cidades, identificar e apresentar em tela a maior e a menor umidade registradas, bem como a(s) data(s) e horário(s) em que essas medições ocorreram.
5. Para cada uma das cidades, identificar e apresentar em tela a maior e a menor pressão atmosférica registradas, bem como a(s) data(s) e horário(s) em que essas medições ocorreram.
6. Calcular e apresentar em tela a média de temperatura, umidade e pressão atmosférica do período para cada uma das cidades.
7. Calcular e apresentar em tela o consumo de bateria em cada uma das cidades, por meio da diferença entre o nível de tensão da bateria no início e no final das medições.
8. Identificar e apresentar em tela os Spreading Factors (SF) utilizados nas transmissões.
9. Registrar logs em arquivo durante o processo de execução do programa, de forma que seja possível realizar auditoria e análise do comportamento do código.
10. Calcular e apresentar em tela o tempo total de execução do programa. Quanto mais rápida a execução, maior será a avaliação desse requisito.
11. Os resultados deverão ser apresentados em tela conforme template apresentado ao final.

# Instruções de postagem

- O código-fonte deverá estar disponível em um repositório GitLab/GitHub, contendo informações sobre sua compilação, execução e demains instruções necessárias para seu correto funcionamento.
- Os resultados devem ser apresentados em um vídeo com duração máxima de 3 minutos, demonstrando o funcionamento do programa e o atendimento de todos os requisitos especificados. Não poderão ser usados recursos para aceleração do vídeo.

# Critérios de avaliação

- Publicação do código no GitLab – 0,5 ponto
- Qualidade do vídeo (clareza, organização e qualidade de áudio e imagem) – 0,5 ponto
- Atendimento dos requisitos especificados – 6,0 pontos

# Template

```plaintext
============================================================
ANÁLISE DE DADOS DOS SENSORES - CityLivingLab
Processamento utilizando pthreads
============================================================

Arquivo analisado: sensores_caxias.json
Total de registros processados: 52.184
Período analisado: 01/01/2025 a 31/12/2025

Arquivo analisado: sensores_bento.json
Total de registros processados: 52.184
Período analisado: 01/01/2025 a 31/12/2025

------------------------------------------------------------
TEMPERATURA (°C)
------------------------------------------------------------
Cidade            | Mínima | Data/Hora             | Máxima | Data/Hora             | Média
-----------------------------------------------------------------------------------------------
Caxias do Sul     | 2.10   | 05/07/2025 06:14:02   | 38.45  | 12/02/2025 15:32:18   | 18.73
Bento Gonçalves   | 1.85   | 06/07/2025 06:10:44   | 36.90  | 14/02/2025 16:02:11   | 17.95


------------------------------------------------------------
UMIDADE (%)
------------------------------------------------------------
Cidade            | Mínima | Data/Hora             | Máxima | Data/Hora             | Média
-----------------------------------------------------------------------------------------------
Caxias do Sul     | 21.30  | 09/01/2025 14:20:10   | 98.40  | 21/06/2025 05:48:03   | 71.22
Bento Gonçalves   | 24.90  | 10/01/2025 13:58:02   | 97.80  | 22/06/2025 06:02:17   | 69.81


------------------------------------------------------------
PRESSÃO ATMOSFÉRICA (hPa)
------------------------------------------------------------
Cidade            | Mínima | Data/Hora             | Máxima | Data/Hora             | Média
-----------------------------------------------------------------------------------------------
Caxias do Sul     | 987.20 | 18/08/2025 03:15:44   | 1024.50| 11/09/2025 11:21:02   | 1007.84
Bento Gonçalves   | 989.10 | 18/08/2025 03:10:15   | 1023.90| 11/09/2025 11:20:03   | 1008.12


------------------------------------------------------------
BATERIA
------------------------------------------------------------
Cidade            | Inicial (V) | Final (V) | Consumo (V)
------------------------------------------------------------
Caxias do Sul     | 3.71        | 3.42      | 0.29
Bento Gonçalves   | 3.69        | 3.51      | 0.18


------------------------------------------------------------
SPREADING FACTORS UTILIZADOS
------------------------------------------------------------
Cidade            | SF utilizados
------------------------------------------------------------
Caxias do Sul     | SF7, SF8, SF9
Bento Gonçalves   | SF7, SF8


------------------------------------------------------------
DESEMPENHO
------------------------------------------------------------
Tempo total de execução: 0.84 segundos
Threads utilizadas: 3
 - Thread 1: leitura dos dados
 - Thread 2: cálculo das estatísticas
 - Thread 3: registro de logs

Arquivo de log gerado: processamento.log

============================================================
Processamento finalizado com sucesso.
============================================================
```
