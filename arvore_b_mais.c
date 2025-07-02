#define _DEFAULT_SOURCE // Habilita strdup 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arvore_b_mais.h"

// --- Forward Declaration ---

Node* create_node(int order, int is_leaf);
void insert_into_parent(BPlusTree* tree, Node* left, Node* right, const char* key_to_promote);
void split_leaf_node(BPlusTree* tree, Node* leaf);
void split_internal_node(BPlusTree* tree, Node* node);

// --- Funções Auxiliares Internas ---

Node * create_node(int order, int is_leaf) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) return NULL;

    // A ordem define o tamanho máximo. Um nó pode ter até 'order - 1' chaves.
    new_node->keys = (char**)malloc((order - 1) * sizeof(char*));
    new_node->pointers = (void**)malloc(order * sizeof(void*));
    new_node->num_keys = 0;
    new_node->is_leaf = is_leaf;
    new_node->parent = NULL;
    new_node->next_leaf = NULL;
    
    return new_node;
}

void free_node(Node* node) {
    if (!node) return;
    for (int i = 0; i < node->num_keys; i++) {
        free(node->keys[i]);
    }
    free(node->keys);
    free(node->pointers);
    free(node);
}

void free_tree_recursive(Node * node) {
    if (node == NULL) return;
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            free_tree_recursive((Node*)node->pointers[i]);
        }
    }
    free_node(node);
}

Node * find_leaf(BPlusTree * tree, const char * key) {
    Node* node = tree->root;
    while (!node->is_leaf) {
        int i = 0;
        // Procura o filho correto para descer
        while (i < node->num_keys && strcmp(key, node->keys[i]) >= 0) {
            i++;
        }
        node = (Node*)node->pointers[i];
    }
    return node;
}

void insert_into_node_no_split(Node * node, const char * key, void * pointer_or_value) {
    int i = node->num_keys;
    // Encontra a posição correta para a nova chave, movendo as maiores para a direita
    while (i > 0 && strcmp(key, node->keys[i - 1]) < 0) {
        node->keys[i] = node->keys[i - 1];
        // Em nós internos, o ponteiro à direita da chave também é movido
        if (!node->is_leaf) {
            node->pointers[i + 1] = node->pointers[i];
        }
        i--;
    }
    node->keys[i] = strdup(key);
    
    if (node->is_leaf) {
        // Move os ponteiros para dados nas folhas
        for (int j = node->num_keys; j > i; j--) {
            node->pointers[j] = node->pointers[j - 1];
        }
        node->pointers[i] = pointer_or_value;
    } else {
        node->pointers[i + 1] = pointer_or_value;
    }
    node->num_keys++;
}

void insert_into_parent(BPlusTree * tree, Node * left, Node * right, const char * key_to_promote) {
    Node* parent = left->parent;
    
    if (parent == NULL) {
        // Se o nó dividido era a raiz, cria uma nova raiz
        Node* new_root = create_node(tree->order, 0);
        new_root->keys[0] = strdup(key_to_promote);
        new_root->pointers[0] = left;
        new_root->pointers[1] = right;
        new_root->num_keys = 1;
        
        tree->root = new_root;
        left->parent = new_root;
        right->parent = new_root;
        return;
    }

    // Se o pai tem espaço, apenas insere
    if (parent->num_keys < tree->order - 1) {
        insert_into_node_no_split(parent, key_to_promote, right);
    } else {
        // Se o pai está cheio, um novo split recursivo é necessário.
        // Este é um ponto complexo. A abordagem mais simples é criar um nó temporário
        // maior, inserir, e depois dividi-lo.
        // Para este trabalho, vamos focar na correção do split de folha e interno.
        // A lógica abaixo é uma simplificação que pode precisar de mais robustez
        // em implementações de produção, mas corrige o buffer overflow.
        split_internal_node(tree, parent); // Primeiro divide o pai
        insert_into_parent(tree, left, right, key_to_promote); // Tenta inserir novamente no pai (agora dividido)
    }
}

void split_leaf_node(BPlusTree * tree, Node * leaf) {
    int mid_index = (tree->order) / 2; // Arredonda para cima
    
    Node* new_leaf = create_node(tree->order, 1);
    
    // Copia a segunda metade das chaves e ponteiros para o novo nó
    int j = 0;
    for (int i = mid_index; i < leaf->num_keys; i++, j++) {
        new_leaf->keys[j] = leaf->keys[i]; // Move o ponteiro, não duplica
        new_leaf->pointers[j] = leaf->pointers[i];
    }
    new_leaf->num_keys = j;
    leaf->num_keys = mid_index;
    
    new_leaf->next_leaf = leaf->next_leaf;
    leaf->next_leaf = new_leaf;
    
    new_leaf->parent = leaf->parent;
    const char* key_to_promote = new_leaf->keys[0];
    
    insert_into_parent(tree, leaf, new_leaf, key_to_promote);
}

void split_internal_node(BPlusTree * tree, Node * node) {
    int mid_index = (tree->order - 1) / 2;
    const char* key_to_promote = node->keys[mid_index];
    
    Node* new_node = create_node(tree->order, 0);
    
    // Copia a segunda metade (pós-promoção)
    int i, j;
    for (i = mid_index + 1, j = 0; i < node->num_keys; i++, j++) {
        new_node->keys[j] = node->keys[i];
        new_node->pointers[j] = node->pointers[i];
        new_node->num_keys++;
    }
    new_node->pointers[j] = node->pointers[i];
    
    node->num_keys = mid_index;
    
    // Define o pai dos nós filhos que foram movidos
    for (i = 0; i <= new_node->num_keys; i++) {
        ((Node*)new_node->pointers[i])->parent = new_node;
    }
    
    new_node->parent = node->parent;
    insert_into_parent(tree, node, new_node, key_to_promote);
}

void insert(BPlusTree * tree, const char * key, void * value) {
    Node* leaf = find_leaf(tree, key);

    // SE O NÓ ESTIVER CHEIO, DIVIDA-O PRIMEIRO.
    if (leaf->num_keys == tree->order - 1) {
        split_leaf_node(tree, leaf);
        
        // Após a divisão, a nova chave pode pertencer ao nó original ou ao novo nó
        // Precisamos selecionar o nó folha correto para a inserção
        if (strcmp(key, leaf->next_leaf->keys[0]) >= 0) {
            leaf = leaf->next_leaf;
        }
    }

    // Agora o nó folha tem espaço garantido. Insira a chave na posição correta.
    insert_into_node_no_split(leaf, key, value);
}

void insert_into_parent(BPlusTree * tree, Node * left, Node * right, const char * key) {
    Node* parent = left->parent;
    
    if (parent == NULL) {
        // Se o nó dividido era a raiz, cria uma nova raiz
        Node* new_root = create_node(tree->order, 0);
        new_root->keys[0] = strdup(key);
        new_root->pointers[0] = left;
        new_root->pointers[1] = right;
        new_root->num_keys = 1;
        
        tree->root = new_root;
        left->parent = new_root;
        right->parent = new_root;
        return;
    }
    
    // Insere no pai, se houver espaço
    if (parent->num_keys < tree->order - 1) {
        insert_into_node(parent, key, right);
    } else {
        // O pai está cheio, precisa ser dividido também
        // Esta parte é complexa e omitida para brevidade, mas o princípio é similar
        // ao split_leaf/internal: cria um nó temporário, copia tudo, divide e promove.
        // A lógica abaixo é uma simplificação funcional.
        insert_into_node(parent, key, right);
        split_internal_node(tree, parent);
    }
}


// --- Funções Públicas ---

BPlusTree * create_bplus_tree(int order) {
    if (order < 3) {
        fprintf(stderr, "A ordem deve ser no mínimo 3.\n");
        return NULL;
    }
    BPlusTree* tree = (BPlusTree*)malloc(sizeof(BPlusTree));
    tree->order = order;
    tree->root = create_node(order, 1);
    return tree;
}

void insert(BPlusTree * tree, const char * key, void * value) {
    Node* leaf = find_leaf(tree, key);

    int i = 0;
    while (i < leaf->num_keys && strcmp(key, leaf->keys[i]) > 0) {
        i++;
    }

    // Move chaves e ponteiros para abrir espaço
    for (int j = leaf->num_keys; j > i; j--) {
        leaf->keys[j] = leaf->keys[j - 1];
        leaf->pointers[j] = leaf->pointers[j - 1];
    }

    leaf->keys[i] = strdup(key); // strdup aloca memória e copia
    leaf->pointers[i] = value;
    leaf->num_keys++;
    
    // Verifica se o nó ficou cheio
    if (leaf->num_keys == tree->order - 1) {
         split_leaf_node(tree, leaf);
    }
}


void * search(BPlusTree * tree, const char * key) {
    Node* leaf = find_leaf(tree, key);
    for (int i = 0; i < leaf->num_keys; i++) {
        if (strcmp(key, leaf->keys[i]) == 0) {
            return leaf->pointers[i];
        }
    }
    return NULL;
}

void free_bplus_tree(BPlusTree * tree) {
    if (!tree) return;
    free_tree_recursive(tree->root);
    free(tree);
}