#include <stdio.h>
#include "visualizacao.h"

void exibir_tracejado(int tipo, int tamanho) {
    char caractere = (tipo == 0) ? '-' : '=';
    for (int i = 0; i < tamanho; i++) {
        printf("%c", caractere);
    }
    printf("\n");
}

void exibir_tracejado_padrao(int tipo) {
    exibir_tracejado(tipo, 60);
}

void exibir_titulo(const char *titulo, int tipo) {
    exibir_tracejado_padrao(tipo);
    printf("%s\n", titulo);
    exibir_tracejado_padrao(tipo);
}

void exibir_tabelas(ESTATISTICAS *cidades, int qtd) {
    const char *metricas[] = {"TEMPERATURA (°C)", "UMIDADE (%)", "PRESSÃO ATMOSFÉRICA (hPa)"};

    for (int m = 0; m < 3; m++) {
        exibir_titulo(metricas[m], 0);
        printf("%-20s | %-7s | %-24s | %-7s | %-24s | %-7s\n",
               "Cidade", "Mínima", "Data/Hora", "Máxima", "Data/Hora", "Média");
        exibir_tracejado(0, 102);

        for (int i = 0; i < qtd; i++) {
            double min, max, media = 0;
            char *dMin, *dMax;

            if (m == 0) {
                min = cidades[i].temperatura.min.valor;
                max = cidades[i].temperatura.max.valor;
                dMin = cidades[i].temperatura.min.tempo;
                dMax = cidades[i].temperatura.max.tempo;
                if (cidades[i].temperatura.cont > 0)
                    media = cidades[i].temperatura.soma / cidades[i].temperatura.cont;
            }
            else if (m == 1) {
                min = cidades[i].umidade.min.valor;
                max = cidades[i].umidade.max.valor;
                dMin = cidades[i].umidade.min.tempo;
                dMax = cidades[i].umidade.max.tempo;
                if (cidades[i].umidade.cont > 0)
                    media = cidades[i].umidade.soma / cidades[i].umidade.cont;
            }
            else {
                min = cidades[i].pressao.min.valor;
                max = cidades[i].pressao.max.valor;
                dMin = cidades[i].pressao.min.tempo;
                dMax = cidades[i].pressao.max.tempo;
                if (cidades[i].pressao.cont > 0)
                    media = cidades[i].pressao.soma / cidades[i].pressao.cont;
            }

            printf("%-21s | %-6.2f | %-22s | %-6.2f | %-22s | %-6.2f\n",
                   cidades[i].nome_cidade, min, dMin, max, dMax, media);
        }
        printf("\n");
    }

    exibir_titulo("BATERIA", 0);
    printf("%-20s | %-11s | %-9s | %-11s\n",
            "Cidade", "Inicial (V)", "Final (V)", "Consumo (V)");
    exibir_tracejado_padrao(0);
    for (int i = 0; i < qtd; i++) {
        printf("%-21s | %-11.2f | %-9.2f | %-11.2f\n",
                cidades[i].nome_cidade, cidades[i].bateria.max.valor, cidades[i].bateria.min.valor, (cidades[i].bateria.max.valor - cidades[i].bateria.min.valor));
    }
    printf("\n");

    exibir_titulo("SPREADING FACTORS (SF)", 0);
    printf("%-20s | %-50s\n",
            "Cidade", "SF utilizados");
    exibir_tracejado_padrao(0);
    for (int i = 0; i < qtd; i++) {
        printf("%-21s | ", cidades[i].nome_cidade);
        if (cidades[i].qtd_sf > 0) {
            int sfs_ordenados[20];
            for (int j = 0; j < cidades[i].qtd_sf; j++) {
                sfs_ordenados[j] = cidades[i].spreading_factors[j];
            }
            ordenar_sfs(sfs_ordenados, cidades[i].qtd_sf);

            for (int j = 0; j < cidades[i].qtd_sf; j++) {
                printf("SF%d", sfs_ordenados[j]);
                if (j < cidades[i].qtd_sf - 1) printf(", ");
            }
        } else {
            printf("Nenhum SF encontrado");
        }
        printf("\n");
    }
    printf("\n");
}
