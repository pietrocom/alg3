# Análise de Desempenho da Árvore B+: O Impacto do Tamanho do Nó

**Disciplina:** Algoritmos III
**Professor:** Andrey Ricardo Pimentel
**Autor:** Pietro Comin (GRR20241955)
**Período:** 2025/1

---

## 📜 Resumo do Projeto

Este projeto investiga o impacto da **ordem (tamanho do nó)** de uma **Árvore B+** no desempenho de busca em um ambiente puramente *in-memory*. O estudo partiu de uma implementação em C e conduziu uma análise experimental para determinar a ordem ótima para diferentes volumes de dados (100, 1.000 e 10.000 registros).

A investigação revelou uma narrativa em duas etapas:
1.  Um primeiro experimento, usando **busca linear** dentro dos nós, produziu resultados contraintuitivos: ordens menores apresentaram desempenho superior, contradizendo a teoria clássica de sistemas baseados em disco.
2.  Um segundo experimento, implementando uma **busca binária** otimizada nos nós, validou uma nova hipótese: o custo computacional da busca interna é um fator crítico, e sua otimização revela o clássico *trade-off* entre a altura da árvore e o custo de processamento do nó, resultando em uma curva de desempenho em "U".

O trabalho conclui que não existe uma "ordem ótima" universal; ela é intrinsecamente dependente do ambiente de execução (disco vs. memória) e, crucialmente, das escolhas algorítmicas internas da própria estrutura de dados.

**Palavras-chave:** Árvore B+, Estrutura de Dados, Análise de Algoritmos, Otimização de Desempenho, Busca Binária, Gerenciamento de Memória.

## 🎯 Objetivo

O objetivo central deste trabalho foi responder à seguinte pergunta: **Em um ambiente totalmente *in-memory*, qual a ordem de uma Árvore B+ que oferece o melhor desempenho de busca?**

A investigação buscou validar ou desafiar a premissa de que ordens maiores são sempre melhores, uma noção derivada de sistemas onde a latência de I/O de disco é o principal gargalo.

## 🔬 Metodologia

O ambiente de teste foi desenvolvido em Linguagem C e dividido em três componentes principais:

1.  **Gerador de Dados (`gerador_dados.c`):** Um programa que cria um arquivo `carros.txt` com 10.000 registros de automóveis, cada um com um RENAVAM (chave) único e atributos gerados aleatoriamente.
2.  **Implementação da Árvore B+ (`arvore_b_mais.c`):** Uma implementação completa da estrutura, com suporte para inserção e busca. A característica principal para este estudo foi a capacidade de alternar entre algoritmos de **busca interna nos nós (Linear e Binária)**.
3.  **Orquestrador do Experimento (`experimento.c`):** Um programa que executa os testes de forma automatizada, iterando sobre:
    * **Algoritmos de busca interna:** Linear e Binária.
    * **Volumes de dados:** 100, 1.000 e 10.000 registros.
    * **Ordens da árvore:** Uma gama de valores (ex: de 5 a 200).

Para cada combinação, o programa popula uma nova árvore e mede o tempo médio de 100 buscas por chaves aleatórias, usando a função `clock()` para aferir o tempo de execução.

## 📈 Resultados e Análise

### Experimento 1: A Surpresa com a Busca Linear

A hipótese inicial, baseada na teoria de I/O, era que ordens maiores (árvores mais "achatadas") resultariam em melhor desempenho. Os resultados foram o oposto:

> **Descoberta:** Com busca linear interna, o desempenho **piorou consistentemente** com o aumento da ordem. Ordens menores foram significativamente mais rápidas.
>
> **Nova Hipótese:** Em memória, o custo computacional para varrer um nó grande (`O(m)`) se torna o gargalo dominante, superando o benefício de uma árvore com menor altura.

### Experimento 2: A Confirmação com a Busca Binária

Para testar a nova hipótese, o algoritmo de busca interna foi otimizado para uma busca binária (`O(log m)`).

> **Confirmação:** A otimização da busca interna alterou drasticamente o perfil de desempenho. Os resultados revelaram uma **curva de desempenho em "U"**, onde uma ordem intermediária oferece o ponto ótimo.
>
> **Análise:** Este resultado valida a hipótese. Com a busca no nó se tornando barata, o *trade-off* entre a altura da árvore e o processamento do nó volta a ser o fator determinante, como previsto pela teoria clássica de algoritmos.

## 結論 Conclusão

Este trabalho demonstrou experimentalmente que o dimensionamento ótimo dos nós de uma Árvore B+ em memória depende criticamente do algoritmo de busca interna utilizado.

1.  **Com Busca Linear:** O custo de varredura do nó é o gargalo. **Ordens pequenas** são preferíveis para minimizar esse custo.
2.  **Com Busca Binária:** O custo de busca no nó é reduzido, revelando o clássico *trade-off* com a altura da árvore. Ordens moderadas, que equilibram os dois fatores, tendem a ser as melhores.

A principal lição é que a otimização de estruturas de dados não pode ser generalizada; ela requer uma análise cuidadosa do ambiente de execução e das escolhas algorítmicas internas.

## 🚀 Como Executar o Projeto

**Pré-requisitos:**
* Um compilador C (ex: GCC).

**Passos:**

1.  **Compilar os programas:**
    ```bash
    # Compilar o gerador de dados
    gcc -o gerador_dados gerador_dados.c -lm

    # Compilar o programa de experimento
    gcc -o experimento experimento.c arvore_b_mais.c -lm
    ```

2.  **Gerar o arquivo de dados:**
    ```bash
    ./gerador_dados
    ```
    Isso criará o arquivo `carros.txt` com 10.000 registros.

3.  **Executar o experimento:**
    ```bash
    ./experimento
    ```
    O programa irá rodar todos os testes e imprimir os resultados de tempo médio de busca no console para cada configuração.

## 📂 Estrutura do Repositório

* `arvore_b_mais.h` / `arvore_b_mais.c`: Arquivos com a implementação da estrutura de dados Árvore B+.
* `gerador_dados.c`: Código-fonte do programa que gera os registros de teste.
* `experimento.c`: Código-fonte do programa que conduz a análise de desempenho.
* `relatorio.pdf`: O relatório detalhado deste trabalho, com tabelas e análises aprofundadas.
* `carros.txt`: Arquivo com 10.000 registros de carros gerado pelo `gerador_dados`.
* `README.md`: Este arquivo.
