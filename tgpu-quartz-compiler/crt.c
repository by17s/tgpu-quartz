#include "crt.h"

#include "stdio.h"
#include "stdlib.h"

#ifdef TARGET_TGPU_QUARTZ
# include "target/tgpu_quartz_gen.c"
# include "target/tgpu_quartz_defs.h"
#else
#error [Err] Invalid target
#endif

typedef struct config_s
{
    int flags;
    
    TARGET_ADDR_T text_base;
    TARGET_ADDR_T data_base;
    TARGET_ADDR_T const_base;
    
} config_t;


list_t* list_new(void) {
    list_t* new_list = (list_t*)malloc(sizeof(list_t));
    if (!new_list) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    new_list->len = 0;
    new_list->last = NULL;
    return new_list;
}


list_node_t* list_new_node(void* value) {
    list_node_t* new_node = (list_node_t*)malloc(sizeof(list_node_t));
    if (!new_node) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    new_node->data = value;
    new_node->next = NULL;
    return new_node;
}

void list_append(list_t *list, void* value) {
    list_node_t* new_node = list_new_node(value);
    list->len += 1;

    if (list->last == NULL) {
        list->last = new_node;
        return;
    }

    list_node_t* current = list->last;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}


void* list_index(list_t* list, size_t index) {
    if (!list || index >= list->len) {
        return NULL;
    }

    list_node_t* current = list->last;
    size_t i = 0;

    while (current != NULL) {
        if (i == index) {
            return current->data;
        }
        current = current->next;
        i++;
    }

    return NULL;
}


void list_free(list_t* list) {
    list_node_t* current = list->last;
    while (current != NULL) {
        list_node_t* temp = current;
        current = current->next;
        free(temp);
    }
}

