/*
 * Simple Dynamic Pointer Array - super simple & naive!
 */

#ifndef CREXX_RXCPDARY_H
#define CREXX_RXCPDARY_H

#include <stdlib.h>

/* Initial Size of the Array - it doubles in size when growing */
#define DPA_INIT_SIZE 5

/* Dynamic Pointer Array */
typedef struct dpa {
    size_t buffer_size;
    size_t size;
    void **pointers;
} dpa;

/* Factory */
static dpa *dpa_f() {
    dpa *new_dpa = malloc(sizeof(dpa));
    new_dpa->size = 0;
    new_dpa->buffer_size = DPA_INIT_SIZE;
    new_dpa->pointers = malloc(sizeof(void*) * new_dpa->buffer_size);
    new_dpa->pointers[0] = 0;
    return new_dpa;
}

/* Add an element (pointer) to the array */
static void dpa_add(dpa *array, void* new_element) {
    array->size++;
    if (array->size + 1 > array->buffer_size) {
        array->buffer_size *= 2;
        array->pointers = realloc(array->pointers, sizeof(void*) * array->buffer_size);
    }
    array->pointers[array->size - 1] = new_element;
    array->pointers[array->size] = 0; /* Null terminate it */
}

/* Add an element (pointer) to the array - making the array ordered */
static void dpa_ado(dpa *array, void* new_element) {
    size_t i;
    void* temp;

    array->size++;
    if (array->size + 1 > array->buffer_size) {
        array->buffer_size *= 2;
        array->pointers = realloc(array->pointers, sizeof(void*) * array->buffer_size);
    }
    array->pointers[array->size - 1] = new_element;
    array->pointers[array->size] = 0; /* Null terminate it */

    /* Ensure the array is ordered */
    i = array->size - 1;
    while (i>0) {
        if (array->pointers[i]<array->pointers[i-1]) {
            /* Swap them */
            temp = array->pointers[i];
            array->pointers[i] = array->pointers[i-1];
            array->pointers[--i] = temp;
        }
        else break;
    }
}

static void free_dpa(dpa *array) {
    free(array->pointers);
    free(array);
}

#endif //CREXX_RXCPDARY_H
