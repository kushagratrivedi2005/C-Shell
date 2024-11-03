#include "signal.h"
#include <stdio.h>
#include "linkedlist.h"
#include <stdlib.h>
#include <unistd.h>
#include "color.h"
#include "command.h"
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

pid_t foreground_pid = -1;  // Definition and initialization of the global variable

extern char current_command[256];  // Buffer to store the current command

// Function to send a signal to a process
void send_signal(pid_t pid, int signal_number) {
    signal_number = signal_number % 32;  // Modulo 32 as per specification

    // Check if the process exists
    if (kill(pid, 0) == -1) {
        if (errno == ESRCH) {
            fprintf(stderr,RED "No such process found\n" RESET);
        } else {
            perror(RED "Error sending signal" RESET);
        }
        return;
    }

    // Send the signal
    if (kill(pid, signal_number) == -1) {
        perror(RED "Error sending signal" RESET);
    } else {
        printf("Sent signal %d to process with pid %d\n", signal_number, pid);
    }
}

// Function to handle Ctrl-C (SIGINT)
void handle_sigint(int signum) {
     if (foreground_pid != -1) {
        if (kill(foreground_pid, SIGINT) == -1) {
            perror(RED "Error sending SIGINT to foreground process" RESET);
        } else {
            printf("Sent SIGINT to foreground process PID: %d\n", foreground_pid);
        }
    } else {
        fprintf(stderr,RED "No foreground process to interrupt\n" RESET);
    }
}

// Function to handle Ctrl-D (logout)
void handle_sigquit(int signum) {
    ProcessNode *current = get_process_list_head();

    // Iterate through the list and send SIGKILL to each process
    while (current) {
        pid_t pid = current->pid;
        if (kill(pid, SIGKILL) == -1) {
            perror(RED "Failed to kill process" RESET);
        } else {
            printf("Sent SIGKILL to process with pid %d\n", pid);
        }
        current = current->next;
    }

    // Free the process list after killing all processes
    free_process_list();

    // Exit the shell
    exit(0);
}

static void background_process_handler(int sig) {
    int status;
    pid_t pid;

    // Wait for all child processes (including the specific one we just stopped)
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        const char *command_name = get_process_name(pid);
        if (WIFEXITED(status)) {
            printf("Background process %d (%s) ended normally with exit status %d\n", pid, command_name, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Background process %d (%s) ended abnormally with signal %d\n", pid, command_name, WTERMSIG(status));
        }
        remove_process(pid);  // Clean up process list if needed
    }
}

// Function to handle Ctrl-Z (SIGTSTP)
void handle_sigtstp(int signum) {
    if (foreground_pid != -1) {
        if (kill(foreground_pid, SIGTSTP) == -1) {
            perror(RED "Error sending SIGTSTP to foreground process" RESET);
        } else {
            printf("Stopped foreground process PID: %d\n", foreground_pid);
            // Store the stopped process as a background process
            add_process(foreground_pid, current_command);
            // Call the background process handler directly for this specific PID
            background_process_handler(SIGCHLD);
        }
    } else {
        fprintf(stderr,RED "No foreground process to stop\n" RESET);
    }
}

// Function to set up signal handlers
void setup_signal_handlers() {
    struct sigaction sa;

    // Handle SIGCHLD for background process management
    sa.sa_handler = background_process_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror(RED "Error setting up SIGCHLD handler" RESET);
        exit(1);
    }

    // Handle Ctrl-C
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror(RED "Error setting up SIGINT handler" RESET);
        exit(1);
    }

    // Handle Ctrl-D
    sa.sa_handler = handle_sigquit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror(RED "Error setting up SIGQUIT handler" RESET);
        exit(1);
    }

    // Handle Ctrl-Z
    sa.sa_handler = handle_sigtstp;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror(RED "Error setting up SIGTSTP handler" RESET);
        exit(1);
    }
}
