#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "color.h"
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PATH_LENGTH 4096  // Increased buffer size for paths

static char log_directory[MAX_PATH_LENGTH] = "";  // Buffer to store the log directory path
static char log_file_path[MAX_PATH_LENGTH] = "";  // Buffer to store the full path to the log file
static char *last_command = NULL;  // Store the last command

void set_log_directory(const char *home_dir) {
    // Calculate the lengths of the home directory and the log directory name
    size_t home_len = strlen(home_dir);
    size_t log_dir_len = strlen(".shell_logs");

    // Ensure the buffer is large enough for the home directory and log directory path
    if (home_len + log_dir_len + 2 >= sizeof(log_directory)) { // +2 for '/' and null terminator
        fprintf(stderr,RED "Error: Home directory path is too long\n" RESET);
        exit(EXIT_FAILURE);
    }

    // Construct the log directory path
    strncpy(log_directory, home_dir, sizeof(log_directory) - 1);
    strncat(log_directory, "/.shell_logs", sizeof(log_directory) - strlen(log_directory) - 1);

    // Calculate the lengths of the log directory path and the log file name
    size_t log_dir_path_len = strlen(log_directory);
    size_t log_file_len = strlen(LOG_FILE);

    // Ensure the buffer is large enough for the full log file path
    if (log_dir_path_len + log_file_len + 2 >= sizeof(log_file_path)) { // +2 for '/' and null terminator
        fprintf(stderr, RED "Error: Log file path is too long\n" RESET);
        exit(EXIT_FAILURE);
    }

    // Construct the log file path
    strncpy(log_file_path, log_directory, sizeof(log_file_path) - 1);
    strncat(log_file_path, "/", sizeof(log_file_path) - strlen(log_file_path) - 1);
    strncat(log_file_path, LOG_FILE, sizeof(log_file_path) - strlen(log_file_path) - 1);

    // Create the log directory if it does not exist
    struct stat st = {0};
    if (stat(log_directory, &st) == -1) {
        if (mkdir(log_directory, 0700) != 0) {
            perror(RED "Error creating log directory" RESET);
            exit(EXIT_FAILURE);
        }
    }
}


// Initialize logging
void init_log() {
    last_command = NULL;
}

// Clean up resources
void cleanup_log() {
    if (last_command != NULL) {
        free(last_command);
        last_command = NULL;
    }
}

// Trim the log file to keep only the most recent entries
void trim_log_file() {
    FILE *log_file = fopen(log_file_path, "r");
    if (log_file == NULL) {
        perror(RED "Error opening log file for trimming" RESET);
        return;
    }

    // Read all lines into a dynamic list
    char *lines[MAX_LOG_ENTRIES];
    size_t num_lines = 0;
    char line[256];

    // Read all lines from the log file
    while (fgets(line, sizeof(line), log_file) != NULL) {
        if (num_lines >= MAX_LOG_ENTRIES) {
            free(lines[num_lines % MAX_LOG_ENTRIES]);
        }
        lines[num_lines % MAX_LOG_ENTRIES] = strdup(line);
        num_lines++;
    }
    
    fclose(log_file);

    // Open file for writing and maintain order
    log_file = fopen(log_file_path, "w");
    if (log_file == NULL) {
        perror(RED "Error opening log file for writing" RESET);
        return;
    }

    // Write the most recent MAX_LOG_ENTRIES lines
    size_t start_index = (num_lines > MAX_LOG_ENTRIES) ? (num_lines % MAX_LOG_ENTRIES) : 0;
    size_t entries_to_write = (num_lines > MAX_LOG_ENTRIES) ? MAX_LOG_ENTRIES : num_lines;

    for (size_t i = 0; i < entries_to_write; i++) {
        fprintf(log_file, "%s", lines[(start_index + i) % MAX_LOG_ENTRIES]);
        free(lines[(start_index + i) % MAX_LOG_ENTRIES]);
    }

    fclose(log_file);
}

// Print the log contents
void print_log() {
    FILE *log_file = fopen(log_file_path, "r");
    if (log_file == NULL) {
        perror(RED "Error opening log file for reading" RESET);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), log_file) != NULL) {
        printf("%s", line);
    }

    fclose(log_file);
}

// Log a new command
void log_command(const char *command) {
    // Check if the command contains "log"
    if (strstr(command, "log") != NULL) {
        return;
    }

    // Check if the command is the same as the last one
    if (last_command != NULL && strcmp(last_command, command) == 0) {
        return;
    }

    // Update the last command
    if (last_command != NULL) {
        free(last_command);
    }
    last_command = strdup(command);

    // Trim the log file if necessary
    trim_log_file();

    // Log the command
    FILE *log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        perror(RED "Error opening log file" RESET);
        return;
    }
    fprintf(log_file, "%s\n", command);
    fclose(log_file);
}

// Purge the log file
void log_purge() {
    FILE *log_file = fopen(log_file_path, "w");
    if (log_file == NULL) {
        perror(RED "Error opening log file for purging" RESET);
        return;
    }

    // Clear the contents of the log file
    fclose(log_file);
}

// Retrieve a command from the log by index
char* get_command_from_log(int index) {
    if (index < 1) {
        fprintf(stderr,RED "Error: Index must be greater than 0.\n" RESET);
        return NULL;
    }

    FILE *log_file = fopen(log_file_path, "r");
    if (log_file == NULL) {
        perror(RED "Error opening log file" RESET);
        return NULL;
    }

    // Count the number of lines in the log file
    size_t num_lines = 0;
    char line[256];
    while (fgets(line, sizeof(line), log_file) != NULL) {
        num_lines++;
    }
    rewind(log_file);

    // Determine the correct number of lines to read
    size_t log_size = (num_lines > MAX_LOG_ENTRIES + 1) ? MAX_LOG_ENTRIES + 1 : num_lines;

    // Allocate an array to store the lines
    char *lines[MAX_LOG_ENTRIES];
    for (size_t i = 0; i < MAX_LOG_ENTRIES; i++) {
        lines[i] = NULL;
    }

    // Read lines from the file into the array
    size_t current_index = 0;
    while (fgets(line, sizeof(line), log_file) != NULL) {
        size_t pos = (current_index) % MAX_LOG_ENTRIES;
        if (lines[pos] != NULL) {
            free(lines[pos]);
        }
        lines[pos] = strdup(line);
        current_index++;
    }
    fclose(log_file);

    // Validate the index
    if (index > log_size) {
        fprintf(stderr,RED "Error: Index exceeds the number of log entries.\n" RESET);
        for (size_t i = 0; i < MAX_LOG_ENTRIES; i++) {
            if (lines[i] != NULL) {
                free(lines[i]);
            }
        }
        return NULL;
    }

    // Calculate the actual index
    size_t actual_index = (log_size - index) % MAX_LOG_ENTRIES;
    char *result = strdup(lines[actual_index]);

    // Remove the trailing newline, if present
    size_t len = strlen(result);
    if (len > 0 && result[len - 1] == '\n') {
        result[len - 1] = '\0';
    }

    // Free allocated memory
    for (size_t i = 0; i < MAX_LOG_ENTRIES; i++) {
        if (lines[i] != NULL) {
            free(lines[i]);
        }
    }

    return result;
}
