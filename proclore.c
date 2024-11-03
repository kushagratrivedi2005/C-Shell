// proclore.c
#include "proclore.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "color.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#define MAX_PATH_LENGTH 4096

static void print_process_info(pid_t pid) {
    char path[MAX_PATH_LENGTH];
    char status[256];
    char exe_path[MAX_PATH_LENGTH];
    char proc_file[MAX_PATH_LENGTH];
    FILE *file;
    ssize_t len;

    // Print PID
    printf("pid : %d\n", pid);

    // Read process status
    snprintf(proc_file, sizeof(proc_file), "/proc/%d/status", pid);
    file = fopen(proc_file, "r");
    if (file) {
        while (fgets(status, sizeof(status), file)) {
            if (strncmp(status, "State:", 6) == 0) {
                printf("process status : %s", status + 7);
                break;
            }
        }
        fclose(file);
    } else {
        perror(RED "Failed to open /proc/[pid]/status" RESET);
    }

    // Print Process Group
    snprintf(proc_file, sizeof(proc_file), "/proc/%d/stat", pid);
    file = fopen(proc_file, "r");
    if (file) {
        int pgid;
        fscanf(file, "%*d %*s %*c %d", &pgid);
        printf("Process Group : %d\n", pgid);
        fclose(file);
    } else {
        perror(RED "Failed to open /proc/[pid]/stat" RESET);
    }

    // Print Virtual Memory
    snprintf(proc_file, sizeof(proc_file), "/proc/%d/statm", pid);
    file = fopen(proc_file, "r");
    if (file) {
        unsigned long vsize;
        fscanf(file, "%lu", &vsize);
        printf("Virtual memory : %lu\n", vsize * getpagesize() / 1024);
        fclose(file);
    } else {
        perror(RED "Failed to open /proc/[pid]/statm" RESET);
    }

    // Print Executable Path
    snprintf(proc_file, sizeof(proc_file), "/proc/%d/exe", pid);
    len = readlink(proc_file, exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        printf("executable path : %s\n", exe_path);
    } else {
        perror(RED "Failed to read /proc/[pid]/exe" RESET);
    }
}

void proclore(const char *pid_str) {
    pid_t pid;

    if (pid_str == NULL || strlen(pid_str) == 0) {
        pid = getpid(); // If no argument is provided, use the shell process ID
    } else {
        pid = (pid_t) atoi(pid_str);
    }

    print_process_info(pid);
}
