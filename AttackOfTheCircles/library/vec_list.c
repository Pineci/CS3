#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "vec_list.h"
#include "vector.h"

VectorList* vec_list_init(size_t initial_size){
    return (VectorList*)list_init(initial_size, free);
}

void vec_list_free(VectorList* list){
    list_free((List*)list);
}

size_t vec_list_size(VectorList *list){
    return list_size((List*)list);
}

Vector vec_list_get(VectorList *list, size_t index){
    return *(Vector*)list_get((List*)list, index);
}

void vec_list_set(VectorList *list, size_t index, Vector value){
    Vector* entry = list_get((List*)list, index);
    *entry = value;
}

void vec_list_add(VectorList *list, Vector value){
    Vector* entry = malloc(sizeof(Vector));
    *entry = value;
    list_add(list, entry);
}

Vector vec_list_remove(VectorList *list){
    Vector* removedPointer = (Vector*)list_remove(list, list_size(list) - 1);
    Vector result = *removedPointer;
    free(removedPointer);
    return result;
}
