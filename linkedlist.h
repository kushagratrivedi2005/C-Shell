#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <sys/types.h>

typedef struct ProcessNode {
    pid_t pid;
    char command[256];
    struct ProcessNode *next;
} ProcessNode;

// Function declarations
void add_process(pid_t pid, const char *command);
void remove_process(pid_t pid);
const char *get_process_name(pid_t pid);
void free_process_list();
void sort_process_list();  // New function to sort the process list
const char* get_process_state(pid_t pid); // New function to get the state of the process
ProcessNode* get_process_list_head();


#endif // LINKEDLIST_H
