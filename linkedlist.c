#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include "color.h"
#include <string.h>
#include <unistd.h>

static ProcessNode *process_list = NULL;

void add_process(pid_t pid, const char *command) {
    ProcessNode *new_node = (ProcessNode *)malloc(sizeof(ProcessNode));
    if (new_node == NULL) {
        perror(RED "malloc failed" RESET);
        return;
    }
    new_node->pid = pid;
    strncpy(new_node->command, command, sizeof(new_node->command) - 1);
    new_node->command[sizeof(new_node->command) - 1] = '\0';
    new_node->next = process_list;
    process_list = new_node;
}

void remove_process(pid_t pid) {
    ProcessNode **current = &process_list;
    while (*current) {
        ProcessNode *entry = *current;
        if (entry->pid == pid) {
            *current = entry->next;
            free(entry);
            return;
        }
        current = &entry->next;
    }
}

ProcessNode* get_process_list_head() {
    return process_list;
}

const char *get_process_name(pid_t pid) {
    ProcessNode *current = process_list;
    while (current) {
        if (current->pid == pid) {
            return current->command;
        }
        current = current->next;
    }
    return "Unknown";
}

void free_process_list() {
    ProcessNode *current = process_list;
    while (current) {
        ProcessNode *temp = current;
        current = current->next;
        free(temp);
    }
    process_list = NULL;
}

// New function to get the state of a process
const char* get_process_state(pid_t pid) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    FILE *file = fopen(path, "r");
    if (!file) return "Unknown";

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "State:", 6) == 0) {
            fclose(file);
            if (strstr(line, "T")) return "Stopped";
            else return "Running";
        }
    }
    fclose(file);
    return "Unknown";
}

// New function to sort the process list by command name (lexicographically)
void sort_process_list() {
    if (!process_list) return;

    // Determine the length of the list
    int length = 0;
    ProcessNode *current = process_list;
    while (current) {
        length++;
        current = current->next;
    }

    // Create an array of pointers to process nodes
    ProcessNode **array = (ProcessNode **)malloc(length * sizeof(ProcessNode *));
    if (!array) {
        perror(RED "malloc failed" RESET);
        return;
    }

    // Fill the array with pointers to the process nodes
    current = process_list;
    for (int i = 0; i < length; i++) {
        array[i] = current;
        current = current->next;
    }

    // Sort the array of pointers by command name using qsort
    int compare_processes(const void *a, const void *b) {
        ProcessNode *nodeA = *(ProcessNode **)a;
        ProcessNode *nodeB = *(ProcessNode **)b;
        return strcmp(nodeA->command, nodeB->command);
    }

    qsort(array, length, sizeof(ProcessNode *), compare_processes);

    // Reconstruct the linked list from the sorted array
    process_list = array[0];
    current = process_list;
    for (int i = 1; i < length; i++) {
        current->next = array[i];
        current = current->next;
    }
    current->next = NULL; // End the list

    // Free the array of pointers
    free(array);
}
