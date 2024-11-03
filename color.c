#include "color.h"
#include <stdio.h>

void print_error(const char *message) {
    fprintf(stderr, "%s%s%s\n", ERROR_COLOR, message, RESET);
}

void print_prompt(const char *message) {
    fprintf(stdout, "%s%s%s", PROMPT_COLOR, message, RESET);
}

void print_file(const char *file_name) {
    fprintf(stdout, "%s%s%s\n", FILE_COLOR, file_name, RESET);
}

void print_dir(const char *dir_name) {
    fprintf(stdout, "%s%s%s\n", DIR_COLOR, dir_name, RESET);
}

void print_exec(const char *exec_name) {
    fprintf(stdout, "%s%s%s\n", EXEC_COLOR, exec_name, RESET);
}
