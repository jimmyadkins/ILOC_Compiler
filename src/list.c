#include "list.h"
#include "utils.h"
#include "ir.h"
#include <stdio.h>
#include <stdlib.h>

// Initialize an empty list
List* emptyList() {
    List* lst = (List*)malloc(sizeof(List));
    assertCondition(lst != NULL, "Failed to allocate memory for empty list");
    lst->head = NULL;
    lst->next = NULL;
    lst->prev = NULL;  // Initialize prev pointer
    lst->tail = lst;
    return lst;
}

// Get the size of the list
int size(List *lst) {
    int count = 0;
    List *current = lst->next; // Skip sentinel node
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}


// Check if the list is empty
bool isEmpty(List* lst) {
    assertCondition(lst != NULL, "List pointer is NULL in isEmpty()");
    return (lst->next == NULL);  // Empty if no elements after sentinel
}

// Print the list
void printList(List *lst) {
    printf("Printing list contents:\n");
    List *current = lst->next;  // Skip sentinel
    int index = 0;

    while (current) {
        printf("Node %d: Opcode = %d, Prev = %p, This = %p, Next = %p\n",
               index,
               current->head ? current->head->opcode : -1,
               (void *)current->prev,
               (void *)current,
               (void *)current->next);
        current = current->next;
        index++;
    }

    printf("List size according to traversal: %d\n", index);
    printf("List size according to size(): %d\n", size(lst));
}

void append(List *lst, IRLine *line) {
    if (!lst || !line) {
        printf("Error: Cannot append to a null list or with a null line.\n");
        return;
    }

    // Create a new node
    List *newNode = (List *)malloc(sizeof(List));
    if (!newNode) {
        printf("Error: Memory allocation failed for new node.\n");
        exit(EXIT_FAILURE);
    }
    newNode->head = line;
    newNode->next = NULL;
    newNode->prev = lst->tail;

    // Update the current tail's next pointer
    lst->tail->next = newNode;

    // Update the list's tail pointer
    lst->tail = newNode;
}
// // Append an IRLine to the list
// void append(List *lst, IRLine *line) {
//     // printf("append\n");
//     if (!lst || !line) {
//         printf("Error: Cannot append to a null list or with a null line.\n");
//         return;
//     }

//     // Create a new node
//     List *newNode = (List *)malloc(sizeof(List));
//     if (!newNode) {
//         printf("Error: Memory allocation failed for new node.\n");
//         exit(EXIT_FAILURE);
//     }
//     newNode->head = line;
//     newNode->next = NULL;
//     newNode->prev = NULL;

//     // Traverse to the last node
//     List *current = lst;
//     while (current->next) {
//         current = current->next;
//     }

//     // Update pointers to include the new node
//     current->next = newNode;
//     newNode->prev = current;

//     // Debug check: Verify the prev pointer
//     if (newNode->prev != current) {
//         printf("Error: Prev pointer mismatch at node. Current = %p, Prev = %p\n",
//                (void *)newNode, (void *)newNode->prev);
//         exit(EXIT_FAILURE);
//     }

//     // Debug log for successful append
//     // printf("Node appended: This = %p, Prev = %p, Next = %p, Opcode = %d\n",
//     //        (void *)newNode,
//     //        (void *)newNode->prev,
//     //        (void *)newNode->next,
//     //        line->opcode);
// }


// Insert an IRLine after a specific node
void insert_after(List *lst, IRLine *line) {
    printf("insert_after_called\n");
    assertCondition(lst != NULL, "List pointer is NULL in insert_after()");
    if (lst->head == NULL) {
        append(lst, line);
        return;
    }

    List *newNode = (List *)malloc(sizeof(List));
    assertCondition(newNode != NULL, "Failed to allocate memory for new list node");
    newNode->head = line;
    newNode->next = lst->next;
    newNode->prev = lst;  // Set the previous pointer

    if (lst->next) {
        lst->next->prev = newNode;  // Update the next node's previous pointer
    }
    lst->next = newNode;
}


// Insert an IRLine at a specific index
void insert_at(List *lst, IRLine *line, int idx) {
    printf("insert_at_called\n");
    assertCondition(lst != NULL, "List pointer is NULL in insert_at()");
    List *newNode = (List *)malloc(sizeof(List));
    assertCondition(newNode != NULL, "Failed to allocate memory for new node");
    newNode->head = line;

    if (idx == 0) {
        newNode->next = lst->next;
        newNode->prev = lst;  // Update the previous pointer
        if (lst->next) {
            lst->next->prev = newNode;
        }
        lst->next = newNode;
        return;
    }

    List *current = lst;
    for (int i = 0; i < idx; ++i) {
        assertCondition(current->next != NULL, "Index out of bounds");
        current = current->next;
    }

    newNode->next = current->next;
    newNode->prev = current;

    if (current->next) {
        current->next->prev = newNode;
    }
    current->next = newNode;
}

// Remove the next node in the list
void remove_next(List *lst) {
    assertCondition(lst != NULL, "List pointer is NULL in remove_next()");
    if (lst->next == NULL) return;

    List *temp = lst->next;
    lst->next = temp->next;

    if (temp->next) {
        temp->next->prev = lst;  // Update the next node's previous pointer
    }

    free(temp->head);
    free(temp);
}

void remove_at(List *lst, int idx) {
    assertCondition(lst != NULL, "List pointer is NULL in remove_at()");

    if (idx == 0) {
        List *to_remove = lst->next;
        if (to_remove == NULL) {
            error("Cannot remove from an empty list.");
        }
        lst->next = to_remove->next;
        if (to_remove->next) {
            to_remove->next->prev = lst;  // Update the next node's previous pointer
        }
        free(to_remove->head);
        free(to_remove);
        return;
    }

    List *current = lst;
    for (int i = 0; i < idx; ++i) {
        assertCondition(current->next != NULL, "Index out of bounds in remove_at()");
        current = current->next;
    }

    List *to_remove = current->next;
    if (to_remove == NULL) {
        error("Index out of bounds in remove_at()");
    }

    current->next = to_remove->next;
    if (to_remove->next) {
        to_remove->next->prev = current;  // Update the next node's previous pointer
    }

    free(to_remove->head);
    free(to_remove);
}


IRLine *getAt(List *lst, int index) {
    assertCondition(lst != NULL, "List pointer is NULL in getAt()");
    int len = size(lst);
    if (index < 0 || index >= len) {
        printf("Error: Index %d out of bounds (list size: %d)\n", index, len);
        return NULL;
    }

    int middle = len / 2;
    List *current;

    if (index <= middle) {
        // Forward traversal
        current = lst->next;  // Skip sentinel
        for (int i = 0; i < index; i++) {
            current = current->next;
        }
    } else {
        // Backward traversal
        current = lst;
        while (current->next) {
            current = current->next;  // Move to the tail
        }
        for (int i = len - 1; i > index; i--) {
            current = current->prev;
        }
    }

    printf("Found node at index %d: Opcode = %d, Address = %p\n",
           index, current->head->opcode, (void *)current);
    return current->head;
}


// Free the entire list
void freeList(List* lst) {
    assertCondition(lst != NULL, "List pointer is NULL in freeList()");
    List* current = lst->next;
    while (current != NULL) {
        List* next = current->next;
        free(current->head);  // Free the IRLine
        free(current);
        current = next;
    }
    free(lst);
}
