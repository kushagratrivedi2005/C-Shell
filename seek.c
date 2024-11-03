#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "color.h"
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include "seek.h"

#define MAX_PATH 1024

// Helper function to print the relative path with appropriate color
static void print_relative_path(const char *base_dir, const char *path, int is_dir) {
    const char *relative_path = path + strlen(base_dir) + 1;
    if (is_dir) {
        printf("\033[1;34m./%s\033[0m\n", relative_path);  // Blue for directories
    } else {
        printf("\033[1;32m./%s\033[0m\n", relative_path);  // Green for files
    }
}

// Helper function to resolve the directory based on the given path
static int resolve_path(char *resolved_path, const char *home_dir, const char *path) {
    char temp_path[MAX_PATH];

    if (strcmp(path, "~") == 0) {
        strcpy(temp_path, home_dir);  // Change to home directory
    } else if (strcmp(path, "..") == 0 || strcmp(path, ".") == 0) {
        // For ".." and ".", simply use the current directory and combine
        if (getcwd(temp_path, sizeof(temp_path)) != NULL) {
            if (strcmp(path, "..") == 0) {
                // Go one directory up
                char *last_slash = strrchr(temp_path, '/');
                if (last_slash != NULL) {
                    *last_slash = '\0';  // Remove the last part of the path
                }
            }
        } else {
            perror(RED "Error getting current directory" RESET);
            return -1;
        }
    } else if (path[0] == '~') {
        snprintf(temp_path, sizeof(temp_path), "%s%s", home_dir, path + 1);  // Change to directory relative to home
    } else {
        strcpy(temp_path, path);  // Treat as an absolute or relative path
    }

    if (realpath(temp_path, resolved_path) == NULL) {
        perror(RED "Error resolving path" RESET);
        return -1;
    }

    return 0;
}

static int search_directory(const char *base_dir, const char *dir_path, const char *search_term, int show_files, int show_dirs, int exact_match, char *result_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror(RED "opendir" RESET);
        return 0;
    }

    struct dirent *entry;
    int match_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == -1) {
            perror(RED "stat" RESET);
            continue;
        }

        int is_match = 0;
        if (exact_match) {
            is_match = strcmp(entry->d_name, search_term) == 0;  // Exact match check
        } else {
            is_match = strstr(entry->d_name, search_term) != NULL;  // Partial match check
        }

        if (is_match) {
            if (S_ISDIR(statbuf.st_mode)) {
                if (show_dirs) {
                    print_relative_path(base_dir, path, 1);
                    strcpy(result_path, path);
                    match_count++;
                }
            } else if (S_ISREG(statbuf.st_mode)) {
                if (show_files) {
                    print_relative_path(base_dir, path, 0);
                    strcpy(result_path, path);
                    match_count++;
                }
            }
        }

        // Recursively search subdirectories
        if (S_ISDIR(statbuf.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            match_count += search_directory(base_dir, path, search_term, show_files, show_dirs, exact_match, result_path);
        }
    }

    closedir(dir);
    return match_count;
}

void seek_command_handler(char **args, int num_args, char *home_dir) {
    int show_files = 1, show_dirs = 1, exact_match = 0;
    char resolved_path[MAX_PATH];
    char *search_term = NULL;
    char *target_dir = ".";

    // Parse flags and arguments
    for (int i = 1; i < num_args; i++) {
        if (args[i][0] == '-') {
            if (strcmp(args[i], "-f") == 0) {
                show_dirs = 0;
            } else if (strcmp(args[i], "-d") == 0) {
                show_files = 0;
            } else if (strcmp(args[i], "-e") == 0) {
                exact_match = 1;
            } else {
                fprintf(stderr,RED "Invalid flag: %s\n" RESET, args[i]);
                return;
            }
        } else {
            if (!search_term) {
                search_term = args[i];
            } else {
                target_dir = args[i];
            }
        }
    }

    if (!search_term) {
        fprintf(stderr, "Usage: seek <flags> <search_term> <target_directory>\n");
        return;
    }

    if (!show_files && !show_dirs) {
        fprintf(stderr,RED "Invalid flags!\n" RESET);
        return;
    }

    // Resolve the target directory path
    if (resolve_path(resolved_path, home_dir, target_dir) != 0) {
        return;
    }

    // Buffer to hold the result path if -e flag is used
    char result_path[MAX_PATH] = {0};

    // Start searching the directory
    int match_count = search_directory(resolved_path, resolved_path, search_term, show_files, show_dirs, exact_match, result_path);

    if (match_count == 0) {
        printf(RED "No match found!\n" RESET);
    } else if (exact_match && match_count == 1) {
        struct stat statbuf;
        if (stat(result_path, &statbuf) == -1) {
            perror(RED "stat" RESET);
            return;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (access(result_path, X_OK) == 0) {
                if (chdir(result_path) == 0) {
                    printf("\033[1;34m%s\033[0m\n", result_path);
                } else {
                    perror(RED "chdir" RESET);
                }
            } else {
                printf(RED "Missing permissions for task!\n" RESET);
            }
        } else if (S_ISREG(statbuf.st_mode)) {
            if (access(result_path, R_OK) == 0) {
                FILE *file = fopen(result_path, "r");
                if (file) {
                    char ch;
                    while ((ch = fgetc(file)) != EOF) {
                        putchar(ch);
                    }
                    fclose(file);
                } else {
                    perror(RED "fopen" RESET);
                }
            } else {
                printf(RED "Missing permissions for task!\n"RESET);
            }
        }
    }
}
