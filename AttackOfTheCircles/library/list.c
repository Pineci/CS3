#include "list.h"
#include <stdlib.h>
#include <assert.h>

List* list_init(size_t initial_size, FreeFunc freeObj){
  assert(initial_size >= 0);
  List* list = malloc(sizeof(List));
  list->data = malloc(sizeof(void*)*initial_size);
  list->capacity = initial_size;
  list->size = 0;
  list->freeObj = freeObj;
  return list;
}

void list_free(List* list){
    if(list->freeObj != NULL){
        for(int i = 0; i < list->size; i++){
          list->freeObj(list->data[i]);
        }
    }
    free(list->data);
    free(list);
}

// list->size is the number of elements in the list, not according to indices.
size_t list_size(List* list){
  return list->size;
}

void* list_get(List* list, size_t index){
  assert(index < list->size);
  return list->data[index];
}

void list_set(List* list, size_t index, void* value){
  assert(index < list->size);
  list->freeObj(list->data[index]);
  list->data[index] = value;
}

void list_increase_capacity(List* list){
  size_t new_capacity = (list->capacity + 1) * 2;
  void** new_data = malloc(sizeof(void*)*new_capacity);
  for(int i = 0; i < list->size; i++){
    new_data[i] = list->data[i];
  }
  free(list->data);
  list->data = new_data;
  list->capacity = new_capacity;
}

void list_add(List* list, void* value){
  if(list->size >= list->capacity){
    list_increase_capacity(list);
  }
  list->data[list->size] = value;
  list->size++;
}

void* list_remove(List* list, size_t index){
  assert(list->size > 0);
  assert(index < list->size);
  void *object = list->data[index];

  for (size_t i = index + 1; i < list->size; i++){
    if (index + 1 < list->size){
      list->data[i - 1] = list->data[i];
    }
  }
  list->size--;
  return object;
}
