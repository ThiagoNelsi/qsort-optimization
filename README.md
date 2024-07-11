## Relatório de Desempenho de Quicksort Otimizado com Inline Assembly

### Introdução
O objetivo deste relatório é comparar a performance de duas implementações do algoritmo Quicksort: a versão original e uma versão otimizada utilizando inline assembly nas funções de comparação e troca de elementos.

### Implementações

1. **Quicksort Original**:
   - Utiliza a função `compare` para comparação de elementos.
   - Utiliza a função `SWAP` para troca de elementos.
   - Implementada em C puro.

2. **Quicksort Otimizado**:
   - Utiliza a função `compare_asm` que faz uso de inline assembly para comparação de elementos.
   - Utiliza a função `SWAP_ASM` que faz uso de inline assembly para troca de elementos.
   - As comparações e trocas entre elementos são realizadas diretamente nos registradores, potencialmente reduzindo o tempo de execução.

### Funções de Comparação

```c
int compare(const void *a, const void *b, void *arg) {
    return (*(int *)a - *(int *)b);
}

int compare_asm(const void *a, const void *b, void *arg) {
    int result;
    __asm__ __volatile__ (
        "movl (%1), %%eax\n\t"
        "movl (%2), %%ebx\n\t"
        "subl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "eax", "ebx"
    );
    return result;
}
```

```c
#define SWAP(a, b, size)                                             \
  do {                                                               \
    size_t __size = (size);                                          \
    char *__a = (a), *__b = (b);                                     \
    do {                                                             \
      char __tmp = *__a;                                             \
      *__a++ = *__b;                                                 \
      *__b++ = __tmp;                                                \
    } while (--__size > 0);                                          \
  } while (0)

#define SWAP_ASM(a, b, size)                                         \
  do {                                                               \
    if ((a) != (b)) {                                                \
      size_t __size = (size);                                        \
      char *__a = (a), *__b = (b);                                   \
      __asm__ __volatile__ (                                         \
        "1:\n\t"                                                     \
        "movb (%1), %%al\n\t"                                        \
        "movb (%2), %%bl\n\t"                                        \
        "movb %%al, (%2)\n\t"                                        \
        "movb %%bl, (%1)\n\t"                                        \
        "incq %1\n\t"                                                \
        "incq %2\n\t"                                                \
        "decq %0\n\t"                                                \
        "jnz 1b\n\t"                                                 \
        : "+r" (__size), "+r" (__a), "+r" (__b)                      \
        :                                                            \
        : "al", "bl", "memory"                                       \
      );                                                             \
    }                                                                \
  } while (0)
```

### Resultados de Performance

Os tempos de execução das duas versões foram medidos e são apresentados abaixo:

- **Tempo do Quicksort Original**: 12.072210 segundos
- **Tempo do Quicksort Otimizado**: 11.289622 segundos

Utilizando o profiler Intel VTune para medir os Hotspots das duas versões, ficou claro a melhora de performance nas versões com inline asm:

![image](https://github.com/ThiagoNelsi/qsort-optimization/assets/52456089/152ca41d-1f0a-4982-86e2-2c3a9f885774)
![image](https://github.com/ThiagoNelsi/qsort-optimization/assets/52456089/28c3ce92-a917-4718-b259-9a26d40be02a)

### Análise

- **Quicksort Original**: A função `compare` consome 17.9% do tempo de CPU total.
- **Quicksort Otimizado**: A função `compare_asm` consome 16.7% do tempo de CPU total, indicando uma melhoria na eficiência da comparação de elementos.
- A redução no tempo de CPU da função de comparação sugere que o uso de inline assembly trouxe benefícios de desempenho, resultando em uma redução do tempo total de execução do Quicksort otimizado.
- A função de troca `SWAP_ASM` também contribuiu para a melhoria geral do desempenho ao otimizar a troca de elementos, reduzindo o overhead das operações de troca.

### Conclusão

A versão otimizada do Quicksort, utilizando inline assembly nas funções de comparação e troca, demonstrou uma melhoria de desempenho em relação à versão original. O tempo de execução foi reduzido de ~12.1 segundos para ~11.2 segundos. A análise dos hotspots confirma que as funções `compare_asm` e `SWAP_ASM` são mais eficientes do que suas contrapartes em C puro, resultando em um uso mais eficiente da CPU.
