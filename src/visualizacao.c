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
                min = cidades[i].min_temp.valor;
                max = cidades[i].max_temp.valor;
                dMin = cidades[i].min_temp.tempo;
                dMax = cidades[i].max_temp.tempo;
                if (cidades[i].cont_temp > 0)
                    media = cidades[i].soma_temp / cidades[i].cont_temp;
            }
            else if (m == 1) {
                min = cidades[i].min_umid.valor;
                max = cidades[i].max_umid.valor;
                dMin = cidades[i].min_umid.tempo;
                dMax = cidades[i].max_umid.tempo;
                if (cidades[i].cont_umid > 0)
                    media = cidades[i].soma_umid / cidades[i].cont_umid;
            }
            else {
                min = cidades[i].min_pres.valor;
                max = cidades[i].max_pres.valor;
                dMin = cidades[i].min_pres.tempo;
                dMax = cidades[i].max_pres.tempo;
                if (cidades[i].cont_pres > 0)
                    media = cidades[i].soma_pres / cidades[i].cont_pres;
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
                cidades[i].nome_cidade, cidades[i].max_batt.valor, cidades[i].min_batt.valor, (cidades[i].max_batt.valor - cidades[i].min_batt.valor));
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
