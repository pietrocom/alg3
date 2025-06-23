# Makefile
# Compilador e flags de compilação
CC=gcc
CFLAGS=-Wall -Wextra -g -std=c99

# --- Diretórios ---
# Diretório dos executáveis
BIN_DIR=bin
# Diretório dos arquivos objeto
OBJ_DIR=obj

# --- Fontes e Objetos ---
# Arquivos fonte para o programa de experimento
EXP_SRCS=arvore_b_mais.c experimento.c
# Deriva os nomes dos arquivos objeto a partir dos fontes, colocando-os no diretório obj/
EXP_OBJS=$(patsubst %.c,$(OBJ_DIR)/%.o,$(EXP_SRCS))

# Arquivo fonte para o gerador de dados
GEN_SRC=gerador_dados.c

# --- Executáveis ---
# Nomes dos executáveis finais no diretório bin/
TARGET_EXP=$(BIN_DIR)/experimento
TARGET_GEN=$(BIN_DIR)/gerador_dados

# --- Regras ---

# Regra padrão: compila todos os alvos, mas não executa.
all: $(TARGET_EXP) $(TARGET_GEN)

# Regra para linkar o executável 'experimento'
$(TARGET_EXP): $(EXP_OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linkando $@"
	$(CC) $(CFLAGS) -o $@ $^

# Regra para compilar e linkar o executável 'gerador_dados'
$(TARGET_GEN): $(GEN_SRC)
	@mkdir -p $(BIN_DIR)
	@echo "Compilando e linkando $@"
	$(CC) $(CFLAGS) -o $@ $<

# Regra genérica para compilar arquivos .c em .o dentro do diretório obj/
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	@echo "Compilando $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

# --- REGRAS DE EXECUÇÃO SEPARADAS ---

# Regra para compilar e executar APENAS o gerador de dados
run-generator: $(TARGET_GEN)
	@echo "Executando o gerador de dados..."
	./$(TARGET_GEN)

# Regra para compilar e executar APENAS o experimento
# ATENÇÃO: O arquivo carros.txt deve existir. Execute 'make run-generator' primeiro.
run-experiment: $(TARGET_EXP)
	@echo "Executando o experimento..."
	./$(TARGET_EXP)

# Regra para limpar todos os arquivos gerados e diretórios
clean:
	@echo "Limpando arquivos de build..."
	rm -rf $(OBJ_DIR) $(BIN_DIR) carros.txt

# Declara regras que não geram arquivos com o mesmo nome
.PHONY: all clean run-generator run-experiment