#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

// Forward declaration of IRLine
struct IRLine;

// Define List structure
typedef struct list {
    struct IRLine *head;  // Pointer to the IRLine
    struct list *next;    // Pointer to the next node
    struct list *prev;    // Pointer to the previous node
    struct list *tail;  // Pointer to the last node
} List;

// Function declarations
List* emptyList();
int size(List *lst);
bool isEmpty(List *lst);
void printList(List *lst);
void append(List *lst, struct IRLine *line);
void insert_after(List *node, struct IRLine *line);
void insert_at(List *lst, struct IRLine *line, int idx);
void remove_next(List *lst);
void remove_at(List *lst, int idx);
struct IRLine *getAt(List *lst, int index);
void freeList(List *lst);

#endif
