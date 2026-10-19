# 🚀 Simulação de Hierarquia de Memória e Localidade de Referência em C

Este repositório contém o código-fonte `localidade.c`, desenvolvido para a disciplina de **Arquitetura de Computadores** com base nos conceitos apresentados no **Capítulo 4 do livro *Organização e Arquitetura de Computadores* (William Stallings, 10ª ed.)**.

O programa realiza um estudo comparativo prático sobre como a **localidade espacial** e a **localidade temporal** impactam o tempo de execução e a taxa de acerto (*hit rate*) da memória cache em quatro padrões de acesso à memória.

---

## 📌 Estrutura do Programa (`localidade.c`)

O programa é dividido em duas partes principais:

### 1️⃣ Parte 1: Medição em Tempo Real (Hardware Real)
Executa a mesma quantidade total de acessos à memória (16.777.216 acessos em uma matriz de 64 MB) sob quatro padrões de acesso distintos, medindo o tempo total de execução via `clock_gettime(CLOCK_MONOTONIC)`:

1. **Por linhas (Alta localidade espacial):** Percorre uma matriz bidimensional contiguamente em ordem *row-major*.
2. **Por colunas (Baixa localidade espacial):** Percorre a matriz saltando $N$ elementos na memória a cada iteração.
3. **Laço pequeno (Alta localidade temporal e espacial):** Reutiliza repetidamente um vetor de 4 KB que cabe inteiramente na Cache L1.
4. **Aleatório (Baixa localidade temporal e espacial):** Acessa posições pseudoaleatórias em um vetor grande via gerador *xorshift*.

> **Nota de Engenharia:** Utiliza uma variável `static volatile` para impedir que o compilador elimine ou reordene os laços durante a otimização (*dead code elimination*).

---

### 2️⃣ Parte 2: Simulador de Memória Cache (Mapeamento Direto)
Simula o comportamento de uma memória cache configurável (padrão: **32 KB de capacidade** e **linhas de 64 Bytes**):
* Modela o cálculo de bloco (`endereco / tam_linha`), linha (`bloco % n_linhas`) e tag (`bloco / n_linhas`).
* Contabiliza acertos (*hits*) e erros (*misses*).
* Calcula o tempo médio de acesso ($T_s$) utilizando a fórmula do **Exemplo 4.1 do Stallings**:
  
  $$\text{Ts} = H \cdot T_1 + (1 - H) \cdot (T_1 + T_2)$$

  *(Onde $T_1 = 0{,}01\,\mu\text{s}$, $T_2 = 0{,}10\,\mu\text{s}$ e $H$ é a taxa de acerto).*

---

## 🛠️ Como Compilar e Executar

### Pré-requisitos
* Compilador C (`gcc` ou `clang`)
* Sistema operacional Linux ou ambiente Bash (WSL/macOS)

### Compilação
O código deve ser compilado com a flag `-O1` (ou inferior) para evitar que o compilador altere o padrão de acesso dos laços:

```bash
gcc -O1 -o localidade localidade.c
