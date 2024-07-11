#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>

/* Byte-wise swap two items of size SIZE. */
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

/* Byte-wise swap two items of size SIZE using inline assembly. */
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

/* Discontinue quicksort algorithm when partition gets below this size. */
#define MAX_THRESH 4

typedef struct {
    char *lo;
    char *hi;
} stack_node;

#define STACK_SIZE (CHAR_BIT * sizeof(size_t))
#define PUSH(low, high) ((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define POP(low, high) ((void) (--top, (low = top->lo), (high = top->hi)))
#define STACK_NOT_EMPTY (stack < top)

void _quicksort(void *const pbase, size_t total_elems, size_t size, int (*cmp)(const void *, const void *, void *), void *arg) {
    char *base_ptr = (char *)pbase;

    const size_t max_thresh = MAX_THRESH * size;

    if (total_elems == 0)
        return;

    if (total_elems > MAX_THRESH) {
        char *lo = base_ptr;
        char *hi = &lo[size * (total_elems - 1)];
        stack_node stack[STACK_SIZE];
        stack_node *top = stack;

        PUSH(NULL, NULL);

        while (STACK_NOT_EMPTY) {
            char *left_ptr;
            char *right_ptr;

            char *mid = lo + size * ((hi - lo) / size >> 1);

            if ((*cmp)((void *)mid, (void *)lo, arg) < 0)
                SWAP(mid, lo, size);
            if ((*cmp)((void *)hi, (void *)mid, arg) < 0)
                SWAP(mid, hi, size);
            else
                goto jump_over;
            if ((*cmp)((void *)mid, (void *)lo, arg) < 0)
                SWAP(mid, lo, size);
        jump_over:;

            left_ptr = lo + size;
            right_ptr = hi - size;

            do {
                while ((*cmp)((void *)left_ptr, (void *)mid, arg) < 0)
                    left_ptr += size;

                while ((*cmp)((void *)mid, (void *)right_ptr, arg) < 0)
                    right_ptr -= size;

                if (left_ptr < right_ptr) {
                    SWAP(left_ptr, right_ptr, size);
                    if (mid == left_ptr)
                        mid = right_ptr;
                    else if (mid == right_ptr)
                        mid = left_ptr;
                    left_ptr += size;
                    right_ptr -= size;
                } else if (left_ptr == right_ptr) {
                    left_ptr += size;
                    right_ptr -= size;
                    break;
                }
            } while (left_ptr <= right_ptr);

            if ((size_t)(right_ptr - lo) <= max_thresh) {
                if ((size_t)(hi - left_ptr) <= max_thresh)
                    POP(lo, hi);
                else
                    lo = left_ptr;
            } else if ((size_t)(hi - left_ptr) <= max_thresh)
                hi = right_ptr;
            else if ((right_ptr - lo) > (hi - left_ptr)) {
                PUSH(lo, right_ptr);
                lo = left_ptr;
            } else {
                PUSH(left_ptr, hi);
                hi = right_ptr;
            }
        }
    }

#define min(x, y) ((x) < (y) ? (x) : (y))

    {
        char *const end_ptr = &base_ptr[size * (total_elems - 1)];
        char *tmp_ptr = base_ptr;
        char *thresh = min(end_ptr, base_ptr + max_thresh);
        char *run_ptr;

        for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
            if ((*cmp)((void *)run_ptr, (void *)tmp_ptr, arg) < 0)
                tmp_ptr = run_ptr;

        if (tmp_ptr != base_ptr)
            SWAP(tmp_ptr, base_ptr, size);

        run_ptr = base_ptr + size;
        while ((run_ptr += size) <= end_ptr) {
            tmp_ptr = run_ptr - size;
            while ((*cmp)((void *)run_ptr, (void *)tmp_ptr, arg) < 0)
                tmp_ptr -= size;

            tmp_ptr += size;
            if (tmp_ptr != run_ptr) {
                char *trav;

                trav = run_ptr + size;
                while (--trav >= run_ptr) {
                    char c = *trav;
                    char *hi, *lo;

                    for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                        *hi = *lo;
                    *hi = c;
                }
            }
        }
    }
}

void _quicksort_optimized(void *const pbase, size_t total_elems, size_t size, int (*cmp)(const void *, const void *, void *), void *arg) {
    char *base_ptr = (char *)pbase;

    const size_t max_thresh = MAX_THRESH * size;

    if (total_elems == 0)
        return;

    if (total_elems > MAX_THRESH) {
        char *lo = base_ptr;
        char *hi = &lo[size * (total_elems - 1)];
        stack_node stack[STACK_SIZE];
        stack_node *top = stack;

        PUSH(NULL, NULL);

        while (STACK_NOT_EMPTY) {
            char *left_ptr;
            char *right_ptr;

            char *mid = lo + size * ((hi - lo) / size >> 1);

            if ((*cmp)((void *)mid, (void *)lo, arg) < 0)
                SWAP_ASM(mid, lo, size);
            if ((*cmp)((void *)hi, (void *)mid, arg) < 0)
                SWAP_ASM(mid, hi, size);
            else
                goto jump_over;
            if ((*cmp)((void *)mid, (void *)lo, arg) < 0)
                SWAP_ASM(mid, lo, size);
        jump_over:;

            left_ptr = lo + size;
            right_ptr = hi - size;

            do {
                while ((*cmp)((void *)left_ptr, (void *)mid, arg) < 0)
                    left_ptr += size;

                while ((*cmp)((void *)mid, (void *)right_ptr, arg) < 0)
                    right_ptr -= size;

                if (left_ptr < right_ptr) {
                    SWAP_ASM(left_ptr, right_ptr, size);
                    if (mid == left_ptr)
                        mid = right_ptr;
                    else if (mid == right_ptr)
                        mid = left_ptr;
                    left_ptr += size;
                    right_ptr -= size;
                } else if (left_ptr == right_ptr) {
                    left_ptr += size;
                    right_ptr -= size;
                    break;
                }
            } while (left_ptr <= right_ptr);

            if ((size_t)(right_ptr - lo) <= max_thresh) {
                if ((size_t)(hi - left_ptr) <= max_thresh)
                    POP(lo, hi);
                else
                    lo = left_ptr;
            } else if ((size_t)(hi - left_ptr) <= max_thresh)
                hi = right_ptr;
            else if ((right_ptr - lo) > (hi - left_ptr)) {
                PUSH(lo, right_ptr);
                lo = left_ptr;
            } else {
                PUSH(left_ptr, hi);
                hi = right_ptr;
            }
        }
    }

#define min(x, y) ((x) < (y) ? (x) : (y))

    {
        char *const end_ptr = &base_ptr[size * (total_elems - 1)];
        char *tmp_ptr = base_ptr;
        char *thresh = min(end_ptr, base_ptr + max_thresh);
        char *run_ptr;

        for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
            if ((*cmp)((void *)run_ptr, (void *)tmp_ptr, arg) < 0)
                tmp_ptr = run_ptr;

        if (tmp_ptr != base_ptr)
            SWAP_ASM(tmp_ptr, base_ptr, size);

        run_ptr = base_ptr + size;
        while ((run_ptr += size) <= end_ptr) {
            tmp_ptr = run_ptr - size;
            while ((*cmp)((void *)run_ptr, (void *)tmp_ptr, arg) < 0)
                tmp_ptr -= size;

            tmp_ptr += size;
            if (tmp_ptr != run_ptr) {
                char *trav;

                trav = run_ptr + size;
                while (--trav >= run_ptr) {
                    char c = *trav;
                    char *hi, *lo;

                    for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                        *hi = *lo;
                    *hi = c;
                }
            }
        }
    }
}

int compare(const void *a, const void *b, void *arg) {
    return (*(int *)a - *(int *)b);
}

int compare_asm(const void *a, const void *b, void *arg) {
    int result;
    __asm__ __volatile__ (
        "movl (%1), %%eax\n\t"   // Move o valor de 'a' para o registrador eax
        "subl (%2), %%eax\n\t"   // Subtrai o valor de 'b' do registrador eax
        "movl %%eax, %0\n\t"     // Move o resultado da subtração para 'result'
        : "=r" (result)          // Output
        : "r" (a), "r" (b)       // Inputs
        : "%eax"                 // Clobbered register
    );
    return result;
}

void fill_array(int *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        arr[i] = rand();
    }
}

void print_array(int *arr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    const size_t array_size = 99000000;
    int *array1 = malloc(array_size * sizeof(int));
    int *array2 = malloc(array_size * sizeof(int));

    srand((unsigned)time(NULL));

    fill_array(array1, array_size);
    memcpy(array2, array1, array_size * sizeof(int));

    clock_t start, end;
    double cpu_time_used;

    start = clock();
    _quicksort(array1, array_size, sizeof(int), compare, NULL);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Original quicksort time: %f seconds\n", cpu_time_used);

    start = clock();
    _quicksort_optimized(array2, array_size, sizeof(int), compare_asm, NULL);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Optimized quicksort time: %f seconds\n", cpu_time_used);

    free(array1);
    free(array2);

    return 0;
}