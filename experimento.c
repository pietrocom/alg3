#define _DEFAULT_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "arvore_b_mais.h"

#define MAX_LINE_LEN 256

typedef struct {
    char* renavam;
    char* dados_completos;
} RegistroCarro;

RegistroCarro * carregar_registros(const char * nome_arquivo, int * num_registros) {
    FILE* fp = fopen(nome_arquivo, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo de registros");
        *num_registros = 0;
        return NULL;
    }
    int count = 0;
    char buffer[MAX_LINE_LEN];
    fgets(buffer, MAX_LINE_LEN, fp);
    while (fgets(buffer, MAX_LINE_LEN, fp)) {
        count++;
    }
    *num_registros = count;
    RegistroCarro* registros = (RegistroCarro*)malloc(count * sizeof(RegistroCarro));
    fseek(fp, 0, SEEK_SET);
    fgets(buffer, MAX_LINE_LEN, fp);
    int i = 0;
    while (fgets(buffer, MAX_LINE_LEN, fp) && i < count) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        registros[i].dados_completos = strdup(buffer);
        char* token = strtok(buffer, ",");
        registros[i].renavam = strdup(token);
        i++;
    }
    fclose(fp);
    return registros;
}

void liberar_registros(RegistroCarro * registros, int num_registros) {
    for (int i = 0; i < num_registros; i++) {
        free(registros[i].renavam);
        free(registros[i].dados_completos);
    }
    free(registros);
}

int main() {
    int ordens_para_testar[] = {5, 10, 25, 50, 75, 100, 150, 200}; 
    int num_ordens = sizeof(ordens_para_testar) / sizeof(ordens_para_testar[0]);

    int tamanhos_de_dados[] = {100, 1000, 10000};
    int num_tamanhos = sizeof(tamanhos_de_dados) / sizeof(tamanhos_de_dados[0]);
    
    int num_buscas = 100;

    int total_registros;
    RegistroCarro* todos_os_registros = carregar_registros("carros.txt", &total_registros);
    if (!todos_os_registros) return 1;

    printf("Iniciando o experimento de desempenho da Arvore B+ em C...\n");
    printf("------------------------------------------------------------\n");
    
    // Loop para alternar entre os modos de busca
    TipoBusca modos_de_busca[] = {BUSCA_LINEAR, BUSCA_BINARIA};
    for (int s = 0; s < 2; s++) {
        TipoBusca modo_atual = modos_de_busca[s];
        printf("\n\n=============== MODO DE BUSCA: %s ===============\n", 
               (modo_atual == BUSCA_LINEAR) ? "LINEAR" : "BINÁRIA");

        for (int i = 0; i < num_tamanhos; i++) {
            int tamanho_atual = tamanhos_de_dados[i];
            if (tamanho_atual > total_registros) continue;

            printf("\n===== TESTANDO COM %d REGISTROS =====\n", tamanho_atual);

            srand(42); // Usa uma semente fixa
            const char* chaves_para_buscar[num_buscas];
            for (int k = 0; k < num_buscas; k++) {
                chaves_para_buscar[k] = todos_os_registros[rand() % tamanho_atual].renavam;
            }

            for (int j = 0; j < num_ordens; j++) {
                int ordem_atual = ordens_para_testar[j];
                
                // Passa o modo de busca para o construtor
                BPlusTree* arvore = create_bplus_tree(ordem_atual, modo_atual);
                
                for (int k = 0; k < tamanho_atual; k++) {
                    insert(arvore, todos_os_registros[k].renavam, todos_os_registros[k].dados_completos);
                }

                clock_t inicio = clock();
                for (int k = 0; k < num_buscas; k++) {
                    search(arvore, chaves_para_buscar[k]);
                }
                clock_t fim = clock();

                double tempo_total = (double)(fim - inicio) / CLOCKS_PER_SEC;
                double tempo_medio = tempo_total / num_buscas;
                
                printf("Ordem: %4d | Tempo medio de busca: %.10f segundos\n", ordem_atual, tempo_medio);

                free_bplus_tree(arvore);
            }
        }
    }

    liberar_registros(todos_os_registros, total_registros);
    printf("\nExperimento concluido.\n");

    return 0;
}