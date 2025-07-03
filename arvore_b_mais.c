#define _DEFAULT_SOURCE // Para strdup
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arvore_b_mais.h"

// --- Protótipos de Funções Internas ---

Node * create_node(int order, int is_leaf);
void free_node(Node * node);
void free_tree_recursive(Node * node);
Node * find_leaf(BPlusTree * tree, const char * key);
void insert_into_leaf(Node * leaf, const char * key, void * value);
void insert_into_parent(BPlusTree * tree, Node * left_child, Node* right_child, const char * key);
void split_leaf_node(BPlusTree*  tree, Node* leaf);


// --- Implementações ---

int find_insertion_point_binary(Node * node, const char * key) {
    int low = 0;
    int high = node->num_keys - 1;
    int mid;
    while (low <= high) {
        mid = low + (high - low) / 2;
        int cmp = strcmp(key, node->keys[mid]);
        if (cmp == 0) {
            return mid;
        } else if (cmp < 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return low; // Retorna a posição onde a chave deveria estar
}

// Cria um novo nó (folha ou interno)
Node * create_node(int order, int is_leaf) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        perror("Falha ao alocar memória para o nó");
        exit(EXIT_FAILURE);
    }
    
    new_node->keys = (char**)malloc((order - 1) * sizeof(char*));
    new_node->pointers = (void**)malloc(order * sizeof(void*));
    if (!new_node->keys || !new_node->pointers) {
        perror("Falha ao alocar memória para chaves/ponteiros do nó");
        exit(EXIT_FAILURE);
    }

    new_node->num_keys = 0;
    new_node->is_leaf = is_leaf;
    new_node->parent = NULL;
    new_node->next_leaf = NULL;
    
    return new_node;
}

// Cria a estrutura da árvore B+
BPlusTree * create_bplus_tree(int order, TipoBusca mode) {
    if (order < 3) {
        fprintf(stderr, "A ordem da árvore deve ser no mínimo 3.\n");
        return NULL;
    }
    BPlusTree* tree = (BPlusTree*)malloc(sizeof(BPlusTree));
    if (!tree) {
        perror("Falha ao alocar memória para a árvore");
        exit(EXIT_FAILURE);
    }
    tree->order = order;
    tree->root = create_node(order, 1);
    tree->search_mode = mode; 
    return tree;
}

// Encontra o nó folha apropriado para uma chave
Node * find_leaf(BPlusTree * tree, const char * key) {
    Node* node = tree->root;
    while (!node->is_leaf) {
        int i;
        if (tree->search_mode == BUSCA_BINARIA) {
            i = find_insertion_point_binary(node, key);
            // Em nós internos, se a chave for >= a encontrada, devemos ir para a direita
            if (i < node->num_keys && strcmp(key, node->keys[i]) >= 0) {
                i++;
            }
        } else { // Busca Linear
            i = 0;
            while (i < node->num_keys && strcmp(key, node->keys[i]) >= 0) {
                i++;
            }
        }
        node = (Node*)node->pointers[i];
    }
    return node;
}

// Busca por uma chave e retorna o valor associado
void * search(BPlusTree * tree, const char * key) {
    Node* leaf = find_leaf(tree, key);
    int i;
    
    if (tree->search_mode == BUSCA_BINARIA) {
        i = find_insertion_point_binary(leaf, key);
        if (i < leaf->num_keys && strcmp(key, leaf->keys[i]) == 0) {
            return leaf->pointers[i];
        }
    } else { // Busca Linear
        for (i = 0; i < leaf->num_keys; i++) {
            if (strcmp(key, leaf->keys[i]) == 0) {
                return leaf->pointers[i];
            }
        }
    }
    
    return NULL;
}

// Insere o par (chave, valor) em um nó folha que garantidamente não está cheio
void insert_into_leaf(Node * leaf, const char * key, void * value) {
    int i = leaf->num_keys;
    while (i > 0 && strcmp(key, leaf->keys[i - 1]) < 0) {
        leaf->keys[i] = leaf->keys[i - 1];
        leaf->pointers[i] = leaf->pointers[i - 1];
        i--;
    }
    
    leaf->keys[i] = strdup(key);
    leaf->pointers[i] = value;
    leaf->num_keys++;
}

// Lógica de divisão do nó folha
void split_leaf_node(BPlusTree * tree, Node * leaf) {
    int mid_index = (tree->order - 1) / 2;
    
    Node* new_leaf = create_node(tree->order, 1);
    
    int j = 0;
    // No esquerdo tem preferência na minha implementação
    for (int i = mid_index; i < tree->order - 1; i++) {
        new_leaf->keys[j] = leaf->keys[i];
        new_leaf->pointers[j] = leaf->pointers[i];
        leaf->keys[i] = NULL;
        j++;
    }
    new_leaf->num_keys = j;
    leaf->num_keys = mid_index;
    
    new_leaf->next_leaf = leaf->next_leaf;
    leaf->next_leaf = new_leaf;
    
    const char* key_to_promote = new_leaf->keys[0];
    
    insert_into_parent(tree, leaf, new_leaf, key_to_promote);
}

// Lógica unificada para inserir no pai
void insert_into_parent(BPlusTree * tree, Node * left_child, Node * right_child, const char * key) {
    Node* parent = left_child->parent;
    
    // Caso 1: O nó dividido era a raiz
    if (parent == NULL) {
        Node* new_root = create_node(tree->order, 0);
        new_root->keys[0] = strdup(key);
        new_root->pointers[0] = left_child;
        new_root->pointers[1] = right_child;
        new_root->num_keys = 1;
        
        tree->root = new_root;
        left_child->parent = new_root;
        right_child->parent = new_root;
        return;
    }
    
    // Caso 2: O pai tem espaço
    if (parent->num_keys < tree->order - 1) {
        int i = parent->num_keys;
        while (i > 0 && strcmp(key, parent->keys[i - 1]) < 0) {
            parent->keys[i] = parent->keys[i - 1];
            parent->pointers[i + 1] = parent->pointers[i];
            i--;
        }
        parent->keys[i] = strdup(key);
        parent->pointers[i + 1] = right_child;
        parent->num_keys++;
        right_child->parent = parent;
    } 
    // Caso 3: O pai está cheio e precisa ser dividido
    else {
        char** temp_keys = malloc(tree->order * sizeof(char*));
        void** temp_pointers = malloc((tree->order + 1) * sizeof(void*));
        int i = 0;

        while (i < tree->order - 1 && strcmp(key, parent->keys[i]) > 0) {
            i++;
        }
        
        memcpy(temp_keys, parent->keys, i * sizeof(char*));
        memcpy(temp_pointers, parent->pointers, (i + 1) * sizeof(void*));
        
        temp_keys[i] = strdup(key);
        temp_pointers[i + 1] = right_child;
        
        memcpy(temp_keys + i + 1, parent->keys + i, (tree->order - 1 - i) * sizeof(char*));
        memcpy(temp_pointers + i + 2, parent->pointers + i + 1, (tree->order - (i + 1)) * sizeof(void*));
        
        int mid_index = tree->order / 2;
        const char* key_to_promote_again = temp_keys[mid_index];
        
        Node* new_internal_node = create_node(tree->order, 0);
        
        parent->num_keys = 0;
        for (int k = 0; k < mid_index; k++) {
            parent->keys[k] = temp_keys[k];
            parent->pointers[k] = temp_pointers[k];
            parent->num_keys++;
        }
        parent->pointers[mid_index] = temp_pointers[mid_index];
        
        for(int k = mid_index; k < tree->order - 1; k++) {
            parent->keys[k] = NULL;
        }

        int j = 0;
        for (int k = mid_index + 1; k < tree->order; k++) {
            new_internal_node->keys[j] = temp_keys[k];
            new_internal_node->pointers[j] = temp_pointers[k];
            ((Node*)new_internal_node->pointers[j])->parent = new_internal_node;
            new_internal_node->num_keys++;
            j++;
        }
        new_internal_node->pointers[j] = temp_pointers[tree->order];
        ((Node*)new_internal_node->pointers[j])->parent = new_internal_node;

        free(temp_keys);
        free(temp_pointers);

        insert_into_parent(tree, parent, new_internal_node, key_to_promote_again);
    }
}

// Lógica de inserção principal
void insert(BPlusTree * tree, const char * key, void * value) {
    Node* leaf = find_leaf(tree, key);

    if (leaf->num_keys == tree->order - 1) {
        split_leaf_node(tree, leaf);
        
        if (strcmp(key, leaf->next_leaf->keys[0]) >= 0) {
            leaf = leaf->next_leaf;
        }
    }

    insert_into_leaf(leaf, key, value);
}

// Libera a memória de um nó
void free_node(Node * node) {
    if (!node) return;
    // Apenas libera as chaves nos nós internos ou em todos se os dados não forem gerenciados externamente
    for (int i = 0; i < node->num_keys; i++) {
        free(node->keys[i]);
    }
    free(node->keys);
    free(node->pointers);
    free(node);
}

// Libera recursivamente todos os nós da árvore
void free_tree_recursive(Node * node) {
    if (node == NULL) return;
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            free_tree_recursive((Node*)node->pointers[i]);
        }
    }
    free_node(node);
}

// Libera toda a memória alocada pela árvore
void free_bplus_tree(BPlusTree * tree) {
    if (!tree) return;
    free_tree_recursive(tree->root);
    free(tree);
}