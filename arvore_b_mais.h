#ifndef ARVORE_B_MAIS_H
#define ARVORE_B_MAIS_H

// Estrutura para um nó da árvore
typedef struct Node {
    int is_leaf;
    int num_keys;
    char** keys;      // Array de chaves (strings)
    void** pointers;  // Array de ponteiros: para outros nós (interno) ou para dados (folha)
    struct Node* parent;
    struct Node* next_leaf; // Ponteiro para a próxima folha (lista ligada)
} Node;

// Estrutura para a árvore B+
typedef struct BPlusTree {
    Node* root;
    int order;
} BPlusTree;

// --- Protótipos das Funções ---

// Cria e retorna uma nova árvore B+
BPlusTree* create_bplus_tree(int order);

// Insere um par (chave, valor) na árvore
void insert(BPlusTree* tree, const char* key, void* value);

// Busca por uma chave e retorna o valor associado
void* search(BPlusTree* tree, const char* key);

// Libera toda a memória alocada pela árvore
void free_bplus_tree(BPlusTree* tree);

#endif