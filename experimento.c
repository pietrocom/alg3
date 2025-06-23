#define _DEFAULT_SOURCE // Habilita strdup 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "arvore_b_mais.h"

#define MAX_LINE_LEN 256

// Estrutura para armazenar os registros lidos do arquivo
typedef struct {
    char* renavam;
    char* dados_completos;
} RegistroCarro;

// Função para carregar registros do arquivo para a memória
RegistroCarro* carregar_registros(const char* nome_arquivo, int* num_registros) {
    FILE* fp = fopen(nome_arquivo, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo de registros");
        *num_registros = 0;
        return NULL;
    }

    // Conta as linhas para alocar memória
    int count = 0;
    char buffer[MAX_LINE_LEN];
    fgets(buffer, MAX_LINE_LEN, fp); // Pula cabeçalho
    while (fgets(buffer, MAX_LINE_LEN, fp)) {
        count++;
    }
    
    *num_registros = count;
    RegistroCarro* registros = (RegistroCarro*)malloc(count * sizeof(RegistroCarro));
    
    // Volta ao início do arquivo e carrega os dados
    fseek(fp, 0, SEEK_SET);
    fgets(buffer, MAX_LINE_LEN, fp); // Pula cabeçalho de novo
    int i = 0;
    while (fgets(buffer, MAX_LINE_LEN, fp) && i < count) {
        buffer[strcspn(buffer, "\r\n")] = 0; // Remove nova linha
        registros[i].dados_completos = strdup(buffer);
        registros[i].renavam = strdup(strtok(buffer, ","));
        i++;
    }
    
    fclose(fp);
    return registros;
}

void liberar_registros(RegistroCarro* registros, int num_registros) {
    for (int i = 0; i < num_registros; i++) {
        free(registros[i].renavam);
        free(registros[i].dados_completos);
    }
    free(registros);
}

int main() {
    int ordens_para_testar[] = {5, 10, 25, 50, 75, 100};
    int num_ordens = sizeof(ordens_para_testar) / sizeof(ordens_para_testar[0]);

    int tamanhos_de_dados[] = {100, 1000, 10000};
    int num_tamanhos = sizeof(tamanhos_de_dados) / sizeof(tamanhos_de_dados[0]);
    
    int num_buscas = 100;

    int total_registros;
    RegistroCarro* todos_os_registros = carregar_registros("carros.txt", &total_registros);
    if (!todos_os_registros) return 1;

    printf("Iniciando o experimento de desempenho da Arvore B+ em C...\n");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < num_tamanhos; i++) {
        int tamanho_atual = tamanhos_de_dados[i];
        if (tamanho_atual > total_registros) continue;

        printf("\n===== TESTANDO COM %d REGISTROS =====\n", tamanho_atual);

        // Seleciona 100 chaves aleatórias para buscar
        const char* chaves_para_buscar[num_buscas];
        for (int k = 0; k < num_buscas; k++) {
            chaves_para_buscar[k] = todos_os_registros[rand() % tamanho_atual].renavam;
        }

        for (int j = 0; j < num_ordens; j++) {
            int ordem_atual = ordens_para_testar[j];

            // 1. Cria e popula a árvore
            BPlusTree* arvore = create_bplus_tree(ordem_atual);
            for (int k = 0; k < tamanho_atual; k++) {
                // O valor inserido é a linha completa do registro
                insert(arvore, todos_os_registros[k].renavam, todos_os_registros[k].dados_completos);
            }

            // 2. Mede o tempo de busca
            clock_t inicio = clock();
            for (int k = 0; k < num_buscas; k++) {
                search(arvore, chaves_para_buscar[k]);
            }
            clock_t fim = clock();

            double tempo_total = (double)(fim - inicio) / CLOCKS_PER_SEC;
            double tempo_medio = tempo_total / num_buscas;
            
            printf("Ordem: %3d | Tempo medio de busca: %.10f segundos\n", ordem_atual, tempo_medio);

            // 3. Libera a memória da árvore para o próximo teste
            free_bplus_tree(arvore);
        }
    }

    // Libera a memória dos registros carregados
    liberar_registros(todos_os_registros, total_registros);
    printf("\nExperimento concluido.\n");

    return 0;
}