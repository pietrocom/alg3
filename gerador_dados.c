#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_REGISTROS 10000
#define RENAVAM_LEN 11

// Função para gerar um RENAVAM aleatório de 11 dígitos
void gerar_renavam(char* buffer) {
    for (int i = 0; i < RENAVAM_LEN; i++) {
        buffer[i] = (rand() % 10) + '0';
    }
    buffer[RENAVAM_LEN] = '\0';
}

// Retorna um ponteiro para um modelo de carro aleatório
const char* gerar_modelo() {
    const char* modelos[] = {"Gol", "Uno", "Palio", "Onix", "HB20", "Kwid", "Mobi", "Argo", "Compass", "Renegade", "T-Cross", "Creta"};
    int num_modelos = sizeof(modelos) / sizeof(modelos[0]);
    return modelos[rand() % num_modelos];
}

// Retorna um ponteiro para uma cor aleatória
const char* gerar_cor() {
    const char* cores[] = {"Preto", "Branco", "Prata", "Vermelho", "Azul", "Cinza"};
    int num_cores = sizeof(cores) / sizeof(cores[0]);
    return cores[rand() % num_cores];
}

// Retorna um ano aleatório
int gerar_ano() {
    return 2000 + (rand() % 25); // Anos entre 2000 e 2024
}

int main() {
    // Semente para o gerador de números aleatórios
    srand(time(NULL));

    FILE* fp = fopen("carros.txt", "w");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo carros.txt");
        return 1;
    }

    fprintf(fp, "renavam,modelo,ano,cor\n"); // Cabeçalho

    // Usar um set simples (array grande) para verificar a unicidade do renavam seria ineficiente.
    // Para 10.000 registros, a chance de colisão é baixa, mas para um sistema real,
    // seria necessário uma estrutura de dados mais robusta. Para este trabalho, vamos assumir
    // que as colisões são raras e não as trataremos para simplificar.
    char renavam[RENAVAM_LEN + 1];
    for (int i = 0; i < NUM_REGISTROS; i++) {
        gerar_renavam(renavam);
        const char* modelo = gerar_modelo();
        int ano = gerar_ano();
        const char* cor = gerar_cor();
        fprintf(fp, "%s,%s,%d,%s\n", renavam, modelo, ano, cor);
    }

    fclose(fp);
    printf("Arquivo 'carros.txt' com %d registros criado com sucesso.\n", NUM_REGISTROS);

    return 0;
}