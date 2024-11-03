#include "neonate.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "color.h"
#include <termios.h>
#include <signal.h>

static struct termios original;

static void restore_terminal_mode(void) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &original) == -1) {
        perror(RED "Failed to restore terminal settings" RESET);
        exit(EXIT_FAILURE);
    }
}

void neonate(int interval) {
    // Set terminal to print mode
    if (tcgetattr(STDIN_FILENO, &original) == -1) {
        perror(RED "Failed to get terminal attributes" RESET);
        exit(EXIT_FAILURE);
    }

    struct termios new = original;
    new.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new) == -1) {
        perror(RED "Failed to set raw mode" RESET);
        exit(EXIT_FAILURE);
    }

    atexit(restore_terminal_mode);

    pid_t child_pid = fork();
    if (child_pid == 0) {
        while (1) {
            FILE *f = fopen("/proc/sys/kernel/ns_last_pid", "r");
            if (!f) {
                perror(RED "Failed to open /proc/sys/kernel/ns_last_pid" RESET);
                exit(EXIT_FAILURE);
            }
            char buffer[15];
            if (fgets(buffer, 15, f)) {
                printf("%s\n", buffer);
            }
            fclose(f);
            sleep(interval);
        }
    } else if (child_pid > 0) {
        char input_char;
        while (read(STDIN_FILENO, &input_char, 1) == 1 && input_char != 'x')
            continue;
        restore_terminal_mode();
        if (kill(child_pid, SIGKILL) == -1) {
            perror(RED "Failed to kill child process" RESET);
            exit(EXIT_FAILURE);
        }
    } else {
        perror(RED "Fork failed" RESET);
        exit(EXIT_FAILURE);
    }
}
